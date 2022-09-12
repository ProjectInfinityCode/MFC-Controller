# MFC-Controller
Arduino code for mass flow controller driver

Arduino code which utilizes a PID algorithm to control the flow of gas in a vacuum chamber. Arduino uses a Pirani gauge to measure pressure, and converts the gauge's 
voltage output to mTorr. From there the PID determines how much voltage to provide a mass flow controller in order to bring the vacuum chamber's pressure to the 
operator's chosen setpoint. This system is intended for fusor applications, but can be used for numerous projects such as chemical vapor deposition, sputtering, etc.  
