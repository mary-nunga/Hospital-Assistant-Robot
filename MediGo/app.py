from flask import Flask, render_template, redirect, url_for
from datetime import datetime
import serial
import threading
import time
import controller

ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)

app = Flask(__name__)


state = {
    "mode": "Idle",            # Idle / Delivery / Waiting for pickup / Spraying
    "distance_cm": 0,
    "tray_status": "Unknown",  # Full / Empty
    "obstacle": False,
    "last_event": "None yet",
}

logs = []


def add_log(message):
    timestamp = datetime.now().strftime("%H:%M:%S")
    logs.insert(0, f"{timestamp}  •  {message}")
    if len(logs) > 50:
        logs.pop()

@app.route("/")
def dashboard():
    return render_template("index.html", state=state, logs=logs)


@app.route("/action/<name>", methods=["POST"])
def action(name):

    if name == "start":
        ser.write(b"START_DELIVERY\n")
        state["mode"] = "Delivery en route"
        state["last_event"] = "Run started"
        add_log("Run started + START_DELIVERY sent")

    elif name == "pause":
        ser.write(b"EMERGENCY_STOP\n")
        state["mode"] = "Paused"
        state["last_event"] = "Robot paused"
        add_log("Robot paused + EMERGENCY_STOP sent")

    elif name == "stop":
        ser.write(b"EMERGENCY_STOP\n")
        state["mode"] = "Idle"
        state["last_event"] = "Robot stopped"
        add_log("Run stopped + EMERGENCY_STOP sent")

    elif name == "spray":
        ser.write(b"AT_RED\n")
        state["mode"] = "Spraying (manual)"
        state["last_event"] = "Spray triggered"
        add_log("Manual spray triggered + AT_RED sent")

    return redirect(url_for("dashboard"))



def read_from_arduino():
    while True:
        try:
            raw = ser.readline().decode(errors='ignore').strip()

            if not raw:
                continue

            # Debug print (optional)
            # print("RAW:", raw)

            # STATUS MESSAGES
            if raw.startswith("STATUS:"):
                msg = raw.replace("STATUS:", "").strip()
                state["last_event"] = msg
                add_log(f"Arduino → {msg}")

                # update dashboard mode
                if msg == "READY":
                    state["mode"] = "Idle"
                elif msg == "DELIVERY_STARTED":
                    state["mode"] = "Delivery en route"
                elif msg == "WAITING_TRAY_EMPTY":
                    state["mode"] = "Waiting for pickup"
                elif msg == "TRAY_EMPTY":
                    state["tray_status"] = "Empty"
                elif msg == "SPRAYING":
                    state["mode"] = "Spraying"
                elif msg == "COMPLETE":
                    state["mode"] = "Idle"

            # DISTANCE
            elif raw.startswith("DIST:"):
                try:
                    dist = int(raw.replace("DIST:", "").strip())
                    state["distance_cm"] = dist
                except:
                    pass

            # OBSTACLE
            elif raw.startswith("OBS:"):
                if "DETECTED" in raw:
                    state["obstacle"] = True
                elif "CLEAR" in raw:
                    state["obstacle"] = False

            # TRAY
            elif raw.startswith("TRAY:"):
                tray_msg = raw.replace("TRAY:", "").strip()
                state["tray_status"] = tray_msg

        except Exception as e:
            print("Error reading serial:", e)

        time.sleep(0.05)


# Start background reader thread
threading.Thread(target=read_from_arduino, daemon=True).start()


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
