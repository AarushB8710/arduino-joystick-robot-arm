Developed a fully functional, four-degree-of-freedom (4-DOF) robotic arm prototype controlled in real-time by an Arduino Uno and a single dual-axis analog joystick. This project explores practical intersections of mechanical prototyping, actuator coordination, and embedded C++ programming.

Key Technical Implementation Details:
• Dual-Mode State Machine: Programmed a single-button control toggle to overcome physical hardware limitations. A quick tap of the joystick button switches the vertical axis control between the Shoulder joint and the Elbow joint. A long press (held for over 0.6 seconds) triggers the Gripper (claw) to open or close, allowing full 4-axis control from a single joystick.
• Incremental Velocity Control: Designed the control algorithm to map analog inputs to speed rather than absolute angles. This ensures the arm moves smoothly without sudden jerks and securely holds its physical position when the joystick is released.
• Physical Prototyping: Fabricated the arm's structural body using double-layered, cross-grained corrugated cardboard to maximize rigidity and reduce twisting. Used toothpick pivots and friction-reducing spacers to maintain stable rotational axes.
• Circuit Diagnostics: Configured live telemetry via the Arduino Serial Monitor to track real-time angles and joint states for calibration and debugging.
