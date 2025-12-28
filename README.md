MAX32666Toy

WARNING: you must patch the MSDK source files for I2C to properly function or you will get hangs in any activity that reads accelerometer data from BMI160.
This is the commit to apply: https://github.com/analogdevicesinc/msdk/commit/1b95b29863b94470d1fa2dd6e4a77c1c5321a901
Note that another commit MUST NOT be present: https://github.com/analogdevicesinc/msdk/commit/b63a90d680048865fe6b24e318ec70204754f14f
If it is, only the first read works, anything after that complains about the bus being busy. Reported as https://github.com/analogdevicesinc/msdk/issues/1487
