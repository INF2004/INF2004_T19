# INF2004_T19

SIT INF2004 (Embedded Systems): C Program for a robot car with the following components:
- RP 2040 Pico W
- Magnetomer (Control the direction of the car)
- 2 Motors (1 each for left and right wheel)
- L298 Motor controller
- IR Sensor (Sidewall detection)
- Ultrasonic Sensor (Obstacle detection)
- Barcode detection and decode (Barcode detection along the maze)
- 2 wheels
- 2 Encoders (1 for each wheel, PID Speed regulator for both wheels to turn at the same time)

# <h3>Workload distribution for the team</h3>

Same as the project proposal submitted with some minor changes, the respective team members split the work equally and did the following:

| Working directory  | Done By |
| ------------- | ------------- |
| Ultrasonic sensor (Distance measurement) - Calculate distance based on time taken for sound waves to bounce back after hitting an object  | Amanda  |
| Infrared sensor (Barcode detection, Sidewall detection) -	Detect and scan barcodes, as well as sidewall detection for collision avoidance  | Hui Ying  |
| Magnetometer sensor (Balance/Direction of the robot) -	Ensure the vehicle stays upright and oriented correctly	 | Brian  |
| Navigation of the robot (Movement and rotation) -	Determine robot position and orientation, make adjustments accordingly  | Ray |
| L298 Motor controller + Encoders (PID) -	Ensures both wheels are turning at the same time  | Ray + Brian  |
| Viewable user interface (User interface) -	The app to operate the robot  | <b>Everyone</b> |
| Integration of all the components  | <b>Everyone</b> |

# <h3>FLOW DIAGRAM</h3>
![Embedded system flow diagram drawio](https://github.com/INF2004/INF2004_T19/assets/41094581/4cae9e96-27df-4d12-b9a0-05ad7d947393)

# <h3>BLOCK DIAGRAM</h3>
![block drawio](https://github.com/INF2004/INF2004_T19/assets/41094581/9cdb7adb-d2f1-46f9-a452-977afab86123)

# <h3>FLOW DIAGRAM</h3>


