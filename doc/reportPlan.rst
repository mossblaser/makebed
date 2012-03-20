Project Report Plan
===================

Topics
------

* What is 3D printing?
* Existing printers
	* Types (FDM, Laser Sintering...)
	* Commercial/DIY (some examples and background)
* Makerbot
	* Brief History/Overview
	* System overview (software, mechanisms)
		* Model -> Slice -> Commands
		* Commands -> Electronics -> Motors
	* Problems with control/electronics
		* Buffer underruns
		* Serial interface
		* Mechanical Relays
		* Many boards
	* Mechanisms
		* Axes
		* Extruder
		* Platform
		* Stepper
		* Heater+Thermistor
		* Endstops(!)
* Existing Electronics
	* RepRap based primarily ATMega/8bit
	* Newer makerbots/repraps use solid state MOSFETs
* ARM uC
	* Faster
	* RTOS
	* Ethernet
	* What's available
		* mbed - readily available + compact
		* LPCExpresso - same but no ethernet
* Electronics Design
	* uC [mbed]
		* IO, Ethernet PHY, 3.3v
		* No debugger!
	* Power [driving/using an ATX PSU]
		* Powering up
		* Standby current limits
		* Issues with 12v
	* Magjack
	* MOSFETs (+ Isolation)
	* Stepper Controllers (Interface, what they do)
	* Thermistors
	* Testing
		* Bleep Test
		* Thermistor testing (IR probe and multimeter)
		* Smoke Test
* Firmware
	* Overview
	* FreeRTOS
		* What and why
		* Base system
	* Stepper control
		* Bresenham
		* What I actually did
		* GPIO
		* Timers
		* Testing (bus pirate, fpga [coming soon?])
	* Temperature control
		* PID (bang bang)
		* Analog In
	* Control (command processing)
		* Testing (test patterns/programmes)
	* GCode
		* Background (and lack of definition)
		* Subset Selection (skeinforge)
		* How its being modelled/implemented
		* Testing (test programmes)
	* Buffer Testing
		* Real utilisation
		* Stalls
	* Networking
		* Interface in general (gcode stream and meta/debug stream)
		* uIP background
		* Testing (wireshark)
			* Driver issues (retransmit hack)
			* TCP Flow-Control Issues
		* UDP Bodge
			* Testing (buffer fill, reliability)
	* Safety (watchdog with temperature/power, reset behaviour, stop button, etc)
	* Desktop Utilities/Debugging
* Printing
	* Initial tests
		* Circle Drawing
			* Increase resolution to test large instruction densities
			* Tests all axes
			* Calibration [as spec is quite well hidden... :P]
		* Heating
			* Heat probe
			* Long duration
			* Calibration (PID Tuning)
	* Calibration
		* Skeinforge Profiles
			* What sort of parameters, workarounds, etc.
		* Test cubes
			* What these test
		* Large models
		* Overhang
		* Bed adherence (due to temperature)
		* Raftless
	* Example prints
	* Performance
		* No noticed buffer problems causing skipped instructions/missed steps
		* Possibly smoother stepper driving [must test]
		* PID needs tuning more but appears largely adequate
		* Mechanical issues remain
* Future work
	* TCP issues (unless I find them!)
	* Z-Stage and other mechanical issues
	* Web interface
	* PWM Heater Control
	* Stepper Extruder
	* Feedback loop for axes


Appendix Candidates
-------------------
* Print job example/"tutorial" (some model?)
	* 3D modelling (openSCAD?) code + gui
	* Slicing (Skeinforge) gui [how to]
	* GCode output (snip the middle...)
	* Sending to the printer (TCP [net cat]/UDP [my util]?)
	* Monitoring the print job
	* Printing
	* Final Product
* Gallery of Printed Items
* Circuit Diagram & [Exact Board Layout - should that be in the report?]
* Gcode (As Implemented) Reference
* Firmware File Reference/Doxygen style manual
* Exact UDP Comms Spec. (should this be in the main report?)

