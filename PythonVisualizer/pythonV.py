import serial
import numpy as np
import cv2

# ser = serial.Serial('COM22', 4000000, timeout=10)  # Adjust port if needed
ser = serial.Serial('COM22', 115200, timeout=10)  # Adjust port if needed

width, height = 50, 50
frame_size = width * height

posx = 0.0
posy = 0.0
alti = 0

print("Waiting for image data...")

while True:
    data = ser.read(frame_size)
    if len(data) != frame_size:
        print("Incomplete frame received")
        continue

    frame = np.frombuffer(data, dtype=np.uint8).reshape((height, width))

    scale = 10
    frame = cv2.resize(frame, (width * scale, height * scale), interpolation=cv2.INTER_NEAREST)


    cv2.imshow("Opti", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

ser.close()
cv2.destroyAllWindows()