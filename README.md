🧠 Digital Signal Generator and Line Coding Visualizer

Course: ITT-036 Programming Assignment 2025
Language: C++17
Platform: macOS
Author: Ansh Chaudhary
Date: 31-10-2025

📘 Overview

This project implements a Digital Signal Generator capable of generating, encoding, and visualizing digital and analog signals using multiple line coding and modulation schemes. It also performs signal analysis such as finding the longest palindrome and longest zero sequence in the data stream.

For extra credit, the project includes a visual waveform decoder that reconstructs the bit stream from the generated signal pattern.

⚙️ Features

✅ Digital and Analog Input Support

User can choose between direct binary input or analog input (PCM / Delta Modulation).

✅ Line Coding Techniques

NRZ-L (Non-Return-to-Zero Level)

NRZ-I (Non-Return-to-Zero Inverted)

Manchester Encoding

Differential Manchester Encoding

AMI (Alternate Mark Inversion)

✅ Scrambling Support

B8ZS (Bipolar 8-Zero Substitution)

HDB3 (High-Density Bipolar of Order 3)

✅ Signal Analysis

Longest Palindrome Detection (Manacher’s Algorithm – O(n) Time Complexity)

Longest Sequence of Zeros (Optimized Scan Algorithm)

✅ Graphical Visualization

Real-time signal plotting using OpenGL (GLUT) framework.

Auto-scaled waveform display with axis labeling and voltage levels.

✅ Extra Credit (+5 Marks)

Optional Decoder from Waveform, analyzing transitions to recover the original digital sequence.

🧩 Libraries and Tools Used
Component	Library / Tool	Purpose
Graphics	OpenGL + GLUT	Rendering waveforms
Compiler	clang++ / g++ (Xcode Command Line Tools)	Compilation
Math	<cmath>	Quantization and normalization
Strings / Input	<cstring>, <iostream>	Input handling
Algorithms	Custom / O(n) palindrome finder	Performance optimization
💻 How to Run (on macOS)

Save the source code
Save your file as:
DigitalSignalGenerator.cpp

Open Terminal and compile

g++ -std=c++17 DigitalSignalGenerator.cpp -o DigitalSignal -framework OpenGL -framework GLUT -DGL_SILENCE_DEPRECATION


Run the program

./DigitalSignal


Follow on-screen instructions:

Choose input type (Digital or Analog)

Select encoding/modulation scheme

Optionally enable scrambling

View waveform visualization in an OpenGL window

📊 Sample Input / Output
Input:
Choice: 1
Enter binary data: 1011001
Encoding: NRZ-I

Output:
Longest Palindrome: 11 (Length: 2)
Encoded Signal: 1 -1 -1 1 1 -1 1
OpenGL waveform displayed in new window.

🧠 Assumptions

Binary input contains only 0 and 1

Analog samples are within [-1.0, +1.0]

AMI is used before scrambling

Manchester and Differential Manchester double the signal length

Visualization scales automatically with bit count

🙌 Acknowledgement

This project was created independently with conceptual guidance from course materials and Forouzan’s Data Communications and Networking.
No external code has been copied; all logic and visual components were written from scratch for this assignment.
Discussions with peers were limited to understanding encoding schemes and OpenGL setup.
