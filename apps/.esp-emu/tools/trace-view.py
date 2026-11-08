#!/usr/bin/env python3
"""Serve the summary views for an esp-emu execution trace.

    esp-emu --chip esp32c6 --firmware app.bin --elf app.elf --trace run.json
    python3 tools/trace-view.py run.json [PORT]

The timeline itself is Perfetto's job — drag `run.json` into
https://ui.perfetto.dev (it is Chrome Trace Event format). This page covers
what a timeline is bad at: where the time actually went, aggregated.

Reads the trace on every request, so refreshing picks up a growing file from
a run still in progress. A trace truncated by a killed run is handled: the
Trace Event format allows an unterminated array, so the closing bracket is
supplied if missing.
"""
import http.server
import json
import socketserver
import sys
from collections import defaultdict

TRACE = sys.argv[1] if len(sys.argv) > 1 else "trace.json"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 8081


def load(path):
    """Parse the trace, tolerating truncation from a killed run."""
    with open(path) as f:
        text = f.read().strip()
    if not text:
        return []
    if not text.endswith("]"):
        text = text.rstrip().rstrip(",") + "]"
    try:
        data = json.loads(text)
    except json.JSONDecodeError as e:
        raise ValueError(f"{path}: not valid trace JSON ({e})") from e
    return data.get("traceEvents", data) if isinstance(data, dict) else data


def summarise(events):
    """Aggregate in one pass, retaining nothing per slice.

    A long run is almost entirely `X` events; keeping them to walk again for
    the span costs hundreds of MB on a multi-hundred-thousand-event trace.
    Everything here is a running total instead.
    """
    names, pcs, proc = {}, [], "?"
    by_tid = defaultdict(lambda: {"us": 0.0, "n": 0})
    nslices, lo, hi = 0, None, None
    for e in events:
        ph = e.get("ph")
        if ph == "X":
            nslices += 1
            t = by_tid[e["tid"]]
            dur = e.get("dur", 0.0)
            t["us"] += dur
            t["n"] += 1
            ts = e["ts"]
            lo = ts if lo is None else min(lo, ts)
            hi = ts + dur if hi is None else max(hi, ts + dur)
        elif ph == "M":
            n = e.get("name")
            if n == "thread_name":
                names[e["tid"]] = e["args"]["name"]
            elif n == "process_name":
                proc = e["args"]["name"]
            elif n == "esp_emu_pc_histogram":
                pcs = e["args"]["samples"]

    # Task names arrive as metadata, so resolve them only once the pass is done.
    per_task = defaultdict(lambda: {"us": 0.0, "n": 0})
    for tid, v in by_tid.items():
        t = per_task[names.get(tid, f"tid{tid}")]
        t["us"] += v["us"]
        t["n"] += v["n"]

    span = (hi - lo) if lo is not None else 0.0
    total = sum(v["us"] for v in per_task.values()) or 1.0
    tasks = sorted(
        ({"task": k, "us": v["us"], "slices": v["n"], "pct": 100.0 * v["us"] / total}
         for k, v in per_task.items()),
        key=lambda r: -r["us"],
    )
    # Aggregate by function, not by PC: a hot loop spans many addresses, and
    # listing each one separately buries the function under its own samples.
    pc_total = sum(p["n"] for p in pcs) or 1
    by_fn = defaultdict(lambda: {"n": 0, "best": -1, "pc": None})
    for p in pcs:
        f = by_fn[p["fn"]]
        f["n"] += p["n"]
        if p["n"] > f["best"]:         # hottest single address in the function
            f["best"], f["pc"] = p["n"], p["pc"]
    hot = sorted(
        ({"fn": k, "n": v["n"], "pc": v["pc"], "pct": 100.0 * v["n"] / pc_total}
         for k, v in by_fn.items()),
        key=lambda r: -r["n"],
    )[:40]
    return {"chip": proc, "span_us": span, "switches": nslices,
            "tasks": tasks, "hot": hot}


PAGE = """<!doctype html><meta charset=utf-8><title>esp-emu trace</title>
<style>
 :root{color-scheme:light dark;--fg:#111;--dim:#666;--line:#d5d5d8;--bar:#4c78a8;--bg:#fff}
 @media(prefers-color-scheme:dark){:root{--fg:#e6e6e6;--dim:#999;--line:#333;--bar:#6ba3d6;--bg:#151517}}
 body{font:14px/1.5 ui-sans-serif,system-ui,sans-serif;margin:0;padding:2rem;
      max-width:60rem;color:var(--fg);background:var(--bg)}
 h1{font-size:1.15rem;margin:0 0 .2rem} h2{font-size:.95rem;margin:2rem 0 .5rem}
 .sub{color:var(--dim);margin-bottom:1.5rem}
 .cards{display:flex;gap:2rem;flex-wrap:wrap;margin-bottom:1rem}
 .card b{display:block;font-size:1.5rem;font-variant-numeric:tabular-nums}
 .card span{color:var(--dim);font-size:.8rem}
 table{border-collapse:collapse;width:100%}
 td,th{text-align:left;padding:.3rem .6rem .3rem 0;border-bottom:1px solid var(--line);
       font-variant-numeric:tabular-nums}
 th{color:var(--dim);font-weight:600;font-size:.78rem;text-transform:uppercase;letter-spacing:.04em}
 td.n,th.n{text-align:right}
 .bar{height:.5rem;background:var(--bar);border-radius:2px;min-width:1px}
 code{font-family:ui-monospace,monospace}
 .empty{color:var(--dim);font-style:italic}
 a{color:var(--bar)}
</style>
<h1>esp-emu trace — <span id=chip></span></h1>
<div class=sub>Timeline: drag <code id=file></code> into
 <a href="https://ui.perfetto.dev" target=_blank>ui.perfetto.dev</a>. Reload to refresh.</div>
<div class=cards id=cards></div>
<h2>CPU time by task</h2><table id=tasks></table>
<h2>Hot functions (sampled PC)</h2><table id=hot></table>
<script>
const f=(n,d=1)=>n.toLocaleString(undefined,{maximumFractionDigits:d});
fetch('data.json').then(r=>r.json()).then(d=>{
  chip.textContent=d.chip; file.textContent=FILE;
  cards.innerHTML=[[f(d.span_us/1000)+' ms','traced'],
                   [f(d.switches,0),'context switches'],
                   [f(d.tasks.length,0),'tasks seen']]
    .map(([v,l])=>`<div class=card><b>${v}</b><span>${l}</span></div>`).join('');
  const rows=(el,head,body)=>el.innerHTML='<tr>'+head+'</tr>'+body;
  rows(tasks,'<th>Task<th class=n>CPU %<th class=n>Time (ms)<th class=n>Slices<th style="width:30%">',
    d.tasks.map(t=>`<tr><td>${t.task}<td class=n>${f(t.pct)}%<td class=n>${f(t.us/1000)}
      <td class=n>${f(t.slices,0)}<td><div class=bar style=width:${t.pct}%></div>`).join('')
    ||'<tr><td class=empty colspan=5>no slices — was --elf given?');
  rows(hot,'<th>Function<th class=n>%<th class=n>Samples<th>PC',
    d.hot.map(h=>`<tr><td><code>${h.fn}</code><td class=n>${f(h.pct)}%
      <td class=n>${f(h.n,0)}<td><code>${h.pc}</code>`).join('')
    ||'<tr><td class=empty colspan=4>no PC samples — --trace-pc-period 0?');
});
</script>
"""


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith("/data.json"):
            try:
                payload = json.dumps(summarise(load(TRACE))).encode()
            except (OSError, ValueError) as e:
                self.send_error(500, str(e))
                return
            body, ctype = payload, "application/json"
        else:
            body = PAGE.replace("FILE", json.dumps(TRACE)).encode()
            ctype = "text/html; charset=utf-8"
        self.send_response(200)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *a):
        pass


class Server(socketserver.ThreadingTCPServer):
    # Matches test_apps/http_throughput/http_server.py: without reuse, a restart
    # inside TIME_WAIT fails, which is the common case for a refresh-as-you-go
    # tool.
    allow_reuse_address = True
    daemon_threads = True


if __name__ == "__main__":
    summarise(load(TRACE))  # fail loudly now, not on the first request
    print(f"Serving {TRACE} on http://localhost:{PORT}")
    Server(("127.0.0.1", PORT), Handler).serve_forever()
