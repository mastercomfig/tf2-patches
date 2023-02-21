import sys

import matplotlib.pyplot as plt
import pandas as pd

if len(sys.argv) < 2:
    print("Usage: analyze <path> [baseline path]")
    sys.exit(1)

baseline = sys.argv[2] if len(sys.argv) > 2 else None

data = pd.read_csv(sys.argv[1], names=["event", "param"])
baseline_data = (
    pd.read_csv(baseline, names=["event", "param"]) if baseline is not None else None
)


def analyze_wakeups(data):
    if data is None:
        return None

    res = {
        "good": {
            "shared": ((data.event == "goodWakeup") & (data.param == "shared")).sum(),
            "direct": ((data.event == "goodWakeup") & (data.param == "direct")).sum(),
            "call": ((data.event == "goodWakeup") & (data.param == "call")).sum(),
        },
        "bad": {
            "cond": ((data.event == "badWakeup") & (data.param == "condCheck")).sum(),
            "pop": (
                (data.event == "badWakeup")
                & ((data.param == "queuePop") | (data.param.isnull()))
            ).sum(),
        },
        "total": 0,
    }

    for v in res["good"].values():
        res["total"] += v
    for v in res["bad"].values():
        res["total"] += v

    return res


wakeups = analyze_wakeups(data)
assert wakeups is not None

wakeups_baseline = analyze_wakeups(baseline_data)


def render_wakeups(keys):
    val = wakeups
    baseline = wakeups_baseline
    for k in keys:
        val = val[k]
        if baseline is not None:
            baseline = baseline[k]

    pct = val / wakeups["total"] * 100

    if baseline is None:
        return [val, f"{pct:.2f}%"]

    pct_change = (val - baseline) / wakeups_baseline["total"] * 100

    return [f"{val} {val - baseline:+}", f"{pct:.2f}% {pct_change:+.2f}%"]


print("Wakeups:")
print(
    pd.DataFrame(
        {
            "Shared task": render_wakeups(["good", "shared"]),
            "Direct task": render_wakeups(["good", "direct"]),
            "Call": render_wakeups(["good", "call"]),
            "Spurious 1": render_wakeups(["bad", "cond"]),
            "Spurious 2": render_wakeups(["bad", "pop"]),
            "Total": [
                f"{wakeups['total']} {wakeups['total'] - wakeups_baseline['total']:+}"
                if wakeups_baseline is not None
                else wakeups["total"],
                f"100% {(wakeups['total'] - wakeups_baseline['total']) / wakeups_baseline['total']*100:+.2f}%"
                if wakeups_baseline is not None
                else "100%",
            ],
        },
    )
)
print()


def render_quantile(dat, base, quant):
    dq = int(dat.quantile(quant))
    if base is None:
        return [dq]

    bq = int(base.quantile(quant))
    return [f"{dq} {dq - bq:+}", f"{(dq - bq) / bq * 100:+.2f}%"]


def render_quantile_table(dat, base):
    return pd.DataFrame(
        {
            "Min": render_quantile(dat, base, 0.00),
            "p10": render_quantile(dat, base, 0.10),
            "p50": render_quantile(dat, base, 0.50),
            "p90": render_quantile(dat, base, 0.90),
            "p95": render_quantile(dat, base, 0.95),
            "p99": render_quantile(dat, base, 0.99),
            "Max": render_quantile(dat, base, 1.00),
        },
    )


jobLatencies = data[data.event == "jobLatency"].param.astype("int64")
jobLatenciesBaseline = (
    baseline_data[baseline_data.event == "jobLatency"].param.astype("int64")
    if baseline_data is not None
    else None
)

jobLatencyP90 = jobLatencies.quantile(0.9)
print("Job Latency:")
print(render_quantile_table(jobLatencies, jobLatenciesBaseline))
print()

waitTimes = data[data.event == "wait"].param.astype("int64")
waitTimesBaseline = (
    baseline_data[baseline_data.event == "wait"].param.astype("int64")
    if baseline_data is not None
    else None
)
waitLatencyP90 = waitTimes.quantile(0.9)
print("Wait Latency:")
print(render_quantile_table(waitTimes, waitTimesBaseline))

fig, axs = plt.subplots(2)
axs[0].set_title("Job Latencies")
axs[0].hist(
    jobLatencies[jobLatencies <= jobLatencyP90],
    density=True,
    cumulative=True,
    histtype="step",
)

axs[1].set_title("Wait Latencies")
axs[1].hist(
    waitTimes[waitTimes <= waitLatencyP90],
    density=True,
    cumulative=True,
    histtype="step",
)
axs[1]

plt.tight_layout()
plt.show()
