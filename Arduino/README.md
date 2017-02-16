# Arduino
Folder containing all the files relating to the microcontroller. 

<b> To do list </b>

1. Split up the code into multiple files and modules. "source.ino" is too bulky at the moment.
2. Port libraries to PIC32 microcontrollers for more power and controls over difference functionalities. Arduino uses 8-bit AVR and is thus not suitable for a more serious project.
3. Organize the codes in examples.

|FOLDER NAME|DESCRIPTION
|-----------|-----------
|source| Sources for controlling <i>IQS316</i> with Arduino Uno R3. Codes are in "Arduino C".
|Tools| Tools from Azoteq website that can be used to poll data from IQS316. Useful for verifying if the microcontroller is working as intended.
|References| Datasheets to the Azoteq IQS316
|Examples| Example codes to perform specific routines using Arduino
