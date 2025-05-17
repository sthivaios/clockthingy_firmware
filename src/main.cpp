#include <Arduino.h>
#include <RTClib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task function prototypes
void SerialPrintDate(void *pvParameters);
TaskHandle_t SerialPrintDateTaskHandle;

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup() {

    Serial.begin(115200);

    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (1) delay(10);
    }

    xTaskCreate(SerialPrintDate, "SerialPrintDate", 4096, nullptr, 5, &SerialPrintDateTaskHandle);
}

void loop() {

}

void SerialPrintDate(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        // Get the current time from the RTC
        DateTime now = rtc.now();

        // Getting each time field in individual variables
        // And adding a leading zero when needed;
        String yearStr = String(now.year(), DEC);
        String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
        String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
        String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC);
        String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
        String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
        String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

        // Complete time string
        String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;

        // Print the complete formatted time
        Serial.println(formattedTime);
        Serial.println();

        vTaskDelay(1000);
    }
}
