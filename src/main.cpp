#include <Arduino.h>
#include <RTClib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task function prototypes
void SerialPrintDate(void *pvParameters);
TaskHandle_t SerialPrintDateTaskHandle;
void SerialInputHandle(void *pvParameters);
TaskHandle_t SerialInputHandleTaskHandle;

// Vars
bool printingTime = true;

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String FormatDateTimeToString(const DateTime &now) {
    String yearStr = String(now.year(), DEC);
    String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
    String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
    String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC);
    String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
    String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
    String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

    // Complete time string
    String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;

    return formattedTime;
}

void setup() {

    Serial.begin(115200);

    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (true) delay(10);
    }


    // Get the current time from the RTC
    DateTime now = rtc.now();
    const String formattedTime = FormatDateTimeToString(now);

    // Print the complete formatted time
    Serial.println(formattedTime);

    xTaskCreate(SerialPrintDate, "SerialPrintDate", 4096, nullptr, 5, &SerialPrintDateTaskHandle);
    xTaskCreate(SerialInputHandle, "SerialInputHandle", 4096, nullptr, 5, &SerialInputHandleTaskHandle);
}

void loop() {

}

void SerialPrintDate(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        if (printingTime) {
            // Get the current time from the RTC
            DateTime now = rtc.now();
            const String formattedTime = FormatDateTimeToString(now);

            // Print the complete formatted time
            Serial.println(formattedTime);

            vTaskDelay(1000);
        } else {
            vTaskDelay(1);
        }
    }
}

void SerialInputHandle(void *pvParameters) {
    (void)pvParameters;

    String input = "";
    // One-time prompt
    Serial.println("\nConfigure time: send YYYY-MM-DD HH:MM:SS");

    for (;;) {
        // Only spin when there's data
        if (Serial.available()) {
            char c = Serial.read();
            printingTime = false;

            // Skip CR if your terminal sends \r\n
            if (c == '\r') {
                continue;
            }

            // End of line: parse!
            if (c == 'E') {
                input.trim();  // strip whitespace/newlines

                int y, mo, d, h, mi, s;
                if (sscanf(input.c_str(), "%d-%d-%d %d:%d:%d",
                           &y, &mo, &d, &h, &mi, &s) == 6) {
                    // adjust your RTC
                    rtc.adjust(DateTime(y, mo, d, h, mi, s));
                    Serial.println("✅ RTC updated!");
                           } else {
                               Serial.println("❌ Invalid format! Use YYYY-MM-DD HH:MM:SS");
                           }

                // reset for next round
                input = "";
                Serial.println("\nConfigure time: send YYYY-MM-DD HH:MM:SS");
                printingTime = true;

            }
            else {
                // accumulate characters
                input += c;
                Serial.print(c);  // echo back if you like
            }
        }

        // give other tasks a chance, and avoid busy-wait
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}