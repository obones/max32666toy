This folder contains the external libraries required for this project, some of which must be grabbed manually.

FastLED:
A checkout of the https://github.com/obones/FastLED/tree/MAX32666 branch should be checked out in a folder named FastLED

LEDMatrix:
A checkout of the https://github.com/Jorgen-VikingGod/LEDMatrix project in a folder named LEDMatrix

GIFReader
A checkout of the https://github.com/obones/AnimatedGIF/tree/max32666 branch in a folder named AnimatedGIF

BMI160 gyro+accel
A checkout of the https://github.com/obones/BMI160-Arduino/tree/max32666 branch in a folder named BMI160-Arduino

https://how2electronics.com/interfacing-bmi160-accelerometer-gyroscope-with-arduino/

sr_delegates
An unzip of the content found in the following article: https://www.codeproject.com/articles/The-Impossibly-Fast-C-Delegates

flatbuffers
 A checkout of the v25.12.19-2026-02-06-03fffb2 tag from the https://github.com/google/flatbuffers project inside a folder named flatbuffers
 Place the flatc.exe binary file from the tagged release inside the flatbuffers folder
 Regenerate the e14toy_interchange.h file with this call, from the src folder:
..\libraries\flatbuffers\flatc --cpp --scoped-enums --gen-all --no-emit-min-max-enum-values --filename-suffix "" e14toy_interchange.fbs

protobuf
https://github.com/Embedded-AMS/EmbeddedProto
https://github.com/nanopb/nanopb

https://en.delphipraxis.net/topic/10027-delphi-support-for-google-protocol-buffers/
https://github.com/HHS-Software/Protocol-Buffer_Delphi_FPC

