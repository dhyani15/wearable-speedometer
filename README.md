# wearable-speedometer
Measuring self selected walking speed is useful in various clinical applications of gait analysis and helps medical practitioners to measure progress of a patient and recommend new therapy based on this data.

I have created an embedded application on STM32 Discovery microcontroller to develop a wearable device that can measure the distance and walking speed using only the gyroscope sensor data.

## Algorithm
I modeled each leg as one single segment, and the two legs are assumed to
be of the same length; thus, while walking when the heel-strikes, the two legs and the distance between the feet formed an isosceles triangle.
The angle between the leg segments was calculated by integrating the angular velocity measurement from the gyroscope attached to the shank, and then the distance between two feet (step length, one half of stride length) was calculated using the properties of the arc length that subtends an angle.
The figure below illustrates the ideas involved in finding the stride length.

## Setup
Setup [PlatformIO](https://platformio.org/) in your Visual Studio Code IDE and connect the [STM board](https://www.st.com/en/evaluation-tools/32f429idiscovery.html) to your computer. Download the repository in your local project folder and compile it to the microcontroller. Once you have compiled it, the lcd display on the board will start highlighting the output. Wear the device near the ankle; start walking and the device will highlight the distance walked every 20 seconds.

## Unique features
I have setup the device in such a way that it will detect the resting phases and walking phases during the 20 second measuring interval and takes that into account when calculating your walking speed and distance travelled.
