#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> // Include Wire library for I2C communication

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(128, 64, &Wire); // Initialize display object with Wire

#define TOUCH_SENSOR_PIN 33

enum ScreenState {
  Animation,
  Status
};

ScreenState currentScreenState = Animation;
unsigned long sittingStartTime = 0; // Variable to store the start time of sitting mode

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Found a MPU-6050 sensor");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {
      delay(10);
    }
  }
  display.display();
  delay(500);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setRotation(0);

  pinMode(TOUCH_SENSOR_PIN, INPUT);
}

void loop() {
  int touchState = digitalRead(TOUCH_SENSOR_PIN);

  if (touchState == HIGH) {
    currentScreenState = Status;
  }

  if (currentScreenState == Animation) {
    displayAnimation();
  } else if (currentScreenState == Status) {
    displayStatus();
  }
}

void displayAnimation() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Animating...");

  int ballSize = 4; // Diameter of the ball
  int ballX = -ballSize; // Initial position of the ball outside the display

  while (ballX < display.width()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    // Draw the ball at the current position
    display.fillCircle(ballX, display.height() / 2, ballSize, WHITE);

    display.display();

    // Move the ball to the right
    ballX++;

    delay(10); // Adjust the delay based on your animation speed
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Animating...");
  delay(1000);
  
  display.display();
}

void displayStatus() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float total_acceleration = sqrt(a.acceleration.x * a.acceleration.x +
                                   a.acceleration.y * a.acceleration.y +
                                   a.acceleration.z * a.acceleration.z);

  String activity;
  if (total_acceleration <= 11.0) {
    activity = "Sitting";
    if (sittingStartTime == 0) {
      sittingStartTime = millis(); // Start the timer when sitting mode is detected
    }
  } else if (total_acceleration > 11.0 && total_acceleration <= 13.0) {
    activity = "Jogging";
    sittingStartTime = 0; // Reset the timer if not in sitting mode
  } else if (total_acceleration > 13.0) {
    activity = "Running";
    sittingStartTime = 0; // Reset the timer if not in sitting mode
  } else {
    activity = "Unknown";
    sittingStartTime = 0; // Reset the timer if not in sitting mode
  }

  display.clearDisplay();
  display.setCursor(0, 0);

  Serial.print("Accelerometer ");
  Serial.print("X: ");
  Serial.print(a.acceleration.x, 1);
  Serial.print(" m/s^2, ");
  Serial.print("Y: ");
  Serial.print(a.acceleration.y, 1);
  Serial.print(" m/s^2, ");
  Serial.print("Z: ");
  Serial.print(a.acceleration.z, 1);
  Serial.println(" m/s^2");

  Serial.print("Total Acc: ");
  Serial.print(total_acceleration, 1);
  Serial.println(" m/s^2");

  display.println("Activity: \n");
  display.println(activity);
  display.display();

  // If in sitting mode for more than 20 seconds, switch back to animation
  if (activity == "Sitting" && (millis() - sittingStartTime) > 20000) {
    currentScreenState = Animation;
    sittingStartTime = 0; // Reset the timer
  }

  delay(1000); // Adjust the delay based on your requirements
}
