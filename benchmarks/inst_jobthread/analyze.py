import sys

import matplotlib.pyplot as plt
import pandas as pd

if len(sys.argv) < 2:
    print("Usage: analyze <path>")
    sys.exit(1)

data = pd.read_csv(sys.argv[1], names=["event", "param"])

goodWakeupShareds = ((data.event == "goodWakup") & (data.param == "shared")).sum()
goodWakeupDirects = ((data.event == "goodWakup") & (data.param == "direct")).sum()
goodWakeupCalls = ((data.event == "goodWakup") & (data.param == "call")).sum()
badWakeups = (data.event == "badWakeup").sum()
totals = goodWakeupShareds + goodWakeupDirects + goodWakeupCalls + badWakeups
print("Wakeups:")
print(
    pd.DataFrame(
        {
            "Shared task": [goodWakeupShareds, f"{goodWakeupShareds/totals*100:.2f}%"],
            "Direct task": [goodWakeupDirects, f"{goodWakeupDirects/totals*100:.2f}%"],
            "Call": [goodWakeupCalls, f"{goodWakeupCalls/totals*100:.2f}%"],
            "Spurious": [badWakeups, f"{badWakeups/totals*100:.2f}%"],
            "Total": [totals, "100%"],
        },
    )
)
print()

jobLatencies = data[data.event == "jobLatency"].param.astype("int64")
jobLatencyP90 = jobLatencies.quantile(0.9)
print("Job Latency:")
print(
    pd.DataFrame(
        {
            "Min": [jobLatencies.quantile(0)],
            "p50": [jobLatencies.quantile(0.5)],
            "p90": [jobLatencyP90],
            "p95": [jobLatencies.quantile(0.95)],
            "p99": [jobLatencies.quantile(0.99)],
            "Max": [jobLatencies.quantile(1)],
        },
    )
)
print()

waitTimes = data[data.event == "wait"].param.astype("int64")
waitLatencyP90 = waitTimes.quantile(0.9)
print("Wait Latency:")
print(
    pd.DataFrame(
        {
            "Min": [waitTimes.quantile(0)],
            "p50": [waitTimes.quantile(0.5)],
            "p90": [waitLatencyP90],
            "p95": [waitTimes.quantile(0.95)],
            "p99": [waitTimes.quantile(0.99)],
            "Max": [waitTimes.quantile(1)],
        },
    )
)

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
