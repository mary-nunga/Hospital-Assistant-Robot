import serial
import threading
import time
import queue
import cv2

# ---- SERIAL SETUP ----
ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
time.sleep(2)

command_queue = queue.Queue()

# robot state data
last_status = "NONE"
distance_cm = 0
obstacle = False
tray_status = "Unknown"

# line following flag
line_following_active = False


# ----- LOW LEVEL SERIAL SEND -----
def send_to_arduino_worker():
    while True:
        cmd = command_queue.get()
        ser.write((cmd + "\n").encode())
        print("Sent:", cmd)


# ----- READ STATUS + SENSORS FROM ARDUINO -----
def read_from_arduino_worker():
    global last_status, distance_cm, obstacle, tray_status

    while True:
        try:
            line = ser.readline().decode(errors='ignore').strip()

            if line.startswith("STATUS:"):
                last_status = line.replace("STATUS:", "").strip()

            elif line.startswith("DIST:"):
                try:
                    distance_cm = int(line.split(":")[1])
                except:
                    pass

            elif line.startswith("OBSTACLE:"):
                obstacle = line.endswith("1")

            elif line.startswith("TRAY:"):
                tray_status = line.split(":")[1]

        except:
            pass

        time.sleep(0.05)


threading.Thread(target=send_to_arduino_worker, daemon=True).start()
threading.Thread(target=read_from_arduino_worker, daemon=True).start()


# ----- PUBLIC API FOR FLASK -----
def send_command(cmd):
    command_queue.put(cmd)

def get_status():
    return last_status

def get_distance():
    return distance_cm

def get_obstacle():
    return obstacle

def get_tray():
    return tray_status


# ======== LINE FOLLOWING ========

def line_follow_loop(duration):
    global line_following_active
    line_following_active = True

    cap = cv2.VideoCapture(0)
    start_time = time.time()

    while line_following_active and (time.time() - start_time < duration):
        ret, frame = cap.read()
        if not ret:
            continue

        # grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, 80, 255, cv2.THRESH_BINARY_INV)

        h, w = thresh.shape
        roi = thresh[int(h*0.7):int(h*0.9), :]

        contours, _ = cv2.findContours(roi, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        if contours:
            c = max(contours, key=cv2.contourArea)
            M = cv2.moments(c)

            if M["m00"] != 0:
                cx = int(M["m10"] / M["m00"])
                center = w // 2

                if cx < center - 40:
                    send_command("MOVE_LEFT")
                elif cx > center + 40:
                    send_command("MOVE_RIGHT")
                else:
                    send_command("MOVE_FORWARD")
        else:
            send_command("STOP")

        time.sleep(0.05)

    # stop motors
    send_command("STOP")


# ======== DELIVERY SEQUENCE ========

def start_delivery_logic():
    global line_following_active
    print(">> Starting delivery")

    send_command("START_DELIVERY")
    time.sleep(1)

    # -- Phase 1: Travel 15 seconds --
    line_follow_loop(15)

    # stop + tell Arduino "we reached destination"
    send_command("AT_BLUE")

    # -- Phase 2: Tray pickup wait --
    timeout = time.time() + 10
    while tray_status != "EMPTY" and time.time() < timeout:
        time.sleep(0.2)

    # if tray still not empty → assume picked
    if tray_status != "EMPTY":
        send_command("TRAY_FORCE_EMPTY")

    # -- Phase 3: Return travel 15 seconds --
    line_follow_loop(15)

    # tell Arduino to spray
    send_command("AT_RED")

    # stop line follower completely
    line_following_active = False


def emergency_stop_logic():
    global line_following_active
    line_following_active = False
    send_command("EMERGENCY_STOP")

