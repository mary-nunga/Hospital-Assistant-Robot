from flask import Flask, render_template, redirect, url_for
from datetime import datetime

app = Flask(__name__)

# ---- fake robot state (for now) ----
state = {
    "mode": "Idle",           # Idle, Delivery, Waiting for pickup, Spraying
    "distance_cm": 0,
    "tray_status": "Unknown", # Full / Empty
    "obstacle": False,
    "last_event": "None yet",
}

logs = []  # simple in-memory log list


def add_log(message):
    logs.insert(0, f"{datetime.now().strftime('%H:%M:%S')}  •  {message}")
    # keep only last 20 for sanity
    if len(logs) > 20:
        logs.pop()


@app.route("/")
def dashboard():
    return render_template("dashboard.html", state=state, logs=logs)


@app.route("/action/<name>", methods=["POST"])
def action(name):
    """Handle button clicks."""
    if name == "start":
        state["mode"] = "Delivery en route"
        state["last_event"] = "Run started"
        add_log("Run started")

    elif name == "pause":
        state["mode"] = "Paused"
        state["last_event"] = "Robot paused"
        add_log("Robot paused")

    elif name == "stop":
        state["mode"] = "Idle"
        state["last_event"] = "Run stopped"
        add_log("Run stopped")

    elif name == "spray":
        state["mode"] = "Spraying (manual)"
        state["last_event"] = "Spray triggered"
        add_log("Spray triggered")

    # these two are for demo, pretending the robot reported events:
    elif name == "fake_delivery_done":
        state["mode"] = "Waiting for spray"
        state["last_event"] = "Delivery complete"
        state["tray_status"] = "Empty"
        add_log("Delivery complete (simulated)")

    elif name == "fake_spray_done":
        state["mode"] = "Idle"
        state["last_event"] = "Spray complete"
        add_log("Spray complete (simulated)")

    return redirect(url_for("dashboard"))


if __name__ == "__main__":
    # debug=True = auto reload when you edit code
    app.run(host="0.0.0.0", port=5000, debug=True)
