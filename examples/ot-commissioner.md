# OT Commissioner CLI
[OpenThread Commissioner](https://openthread.io/guides/commissioner/build) not working in WSL
Its commands can be found [here](https://github.com/openthread/ot-commissioner/blob/main/src/app/cli/README.md).
https://openthread.io/guides/border-router/external-commissioning/cli

# OTBR Commissioner CLI:
~~~
commissioner help
~~~
> announce
> energy
> id
> joiner
> mgmtget
> mgmtset
> panid
> provisioningurl
> sessionid
> start
> state
> stop

End Device:
~~~
eui64
~~~
> 404ccafffe58101c

??? OTBR:
~~~
commissioner joiner enable meshcop 404ccafffe58101c
commissioner joiner enableall meshcop J01NU5
commissioner joiner enablea meshcop J01NU5
~~~
> Error 7: InvalidArgs
