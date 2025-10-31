@echo off

set token="eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI0MTQ5YzlmMDllMTA0MTg4YjRkYWY4Nzc4YWQxZTdhMiIsImlhdCI6MTc2MTgwOTI2NSwiZXhwIjoyMDc3MTY5MjY1fQ.FUJ7NOXYP0BrU_6o3syIy9H4UFlJOfCvT7BO0307pmE"

curl http://192.168.1.101:8123/api/config -H "Content-Type: application/json" -H "Authorization: Bearer %token%" > _api-config.json
