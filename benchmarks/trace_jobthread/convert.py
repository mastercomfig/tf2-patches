import json
import sys
from pathlib import Path

if len(sys.argv) < 2:
    print("Usage: analyze <path>")
    sys.exit(1)

data = []

src = Path(sys.argv[1])
for l in src.open("r"):
    data.append(json.loads(l.replace("\\", "\\\\")))

thread_to_pool = {}
thread_to_pool[9999] = 9999

start_ts = data[0]["t"]
for x in data:
    start_ts = min(start_ts, x["t"])

    if x["e"] == "new_thread":
        thread_to_pool[x["thread_id"]] = x["pool_id"]


def convert(x):
    res = {
        "ts": (x["t"] - start_ts) // 1000,
        "name": x["e"],
        "ph": "i",
        "cat": "all",
        "pid": 9999,
        "tid": 9999,
        "args": {},
    }

    if x["e"] == "new_thread_pool":
        res["pid"] = x["pool_id"]
        return [
            {**res},
            {
                "pid": res["pid"],
                "ph": "M",
                "name": "process_name",
                "args": {"name": x["name"]},
            },
        ]

    if x["e"] == "new_job":
        # return []
        res["name"] = x["name"] if x["name"] != "" else f"job {x['job_id']}"
        res["tid"] = 9999
        res["ph"] = "B"
        res["pid"] = x["pool_id"]
        res["args"] = {"id": x["job_id"]}

    if x["e"] == "start_job":
        res["name"] = x["name"] if x["name"] != "" else f"job {x['job_id']}"
        res["tid"] = x["thread_id"]
        res["ph"] = "B"
        res["pid"] = thread_to_pool[x["thread_id"]]
        res["args"] = {"id": x["job_id"]}

        return [
            {
                **res,
                "ts": res["ts"],
                "ph": "E",
                "tid": 9999,
            },
            res,
        ]

    if x["e"] == "finish_job":
        res["name"] = x["name"] if x["name"] != "" else f"job {x['job_id']}"
        res["tid"] = x["thread_id"]
        res["ph"] = "E"
        res["pid"] = thread_to_pool[res["tid"]]
        res["args"] = {"id": x["job_id"]}

    if x["e"] == "start_wait":
        res["name"] = "wait"
        res["tid"] = x["thread_id"]
        res["ph"] = "B"
        res["pid"] = thread_to_pool[x["thread_id"]]
    if x["e"] == "done_wait":
        res["name"] = "wait"
        res["tid"] = x["thread_id"]
        res["ph"] = "E"
        res["pid"] = thread_to_pool[x["thread_id"]]

    if x["e"] == "yield_wait_start":
        res["name"] = x["name"] if x["name"] != "" else f"job {x['job_id']}"
        res["tid"] = 9999
        res["ph"] = "E"
        res["pid"] = x["pool_id"]
        res["args"] = {"id": x["job_id"]}

    return [res]


traceEvents = []
for x in data:
    traceEvents.extend(convert(x))

src.with_suffix(".json").write_text(
    json.dumps(
        {
            "traceEvents": traceEvents,
            "displayTimeUnit": "ns",
        },
        # indent=2,
    )
)
