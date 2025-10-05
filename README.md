# An Open-Source Data Acquisition Software
Across nearly every scientific endeavor, there is a need to design experiments, collect data, and perform analysis on that data in order to test or develop a hypothesis. In modern times, this almost always involves the use of computerized a wide variety of processes, and specialised sensors are used to quantify and collect it. The relevant
data from a process or experiment is collected via sensors, and is ported (usually) through a USB port, where the sensor data is interpreted in a specialised Data Acquisition Software (DAS) such as LabVIEW. The process itself can be considered a "solved problem", but there is a limitation that exists in the current selection of such software: All of the main software available for practical use is highly proprietary,
with tightly implemented vendor lock-in. This software is often very expensive, and is not available for purchase, but rather locked behind a subscription licensing scheme. Additionally, the current software
available is either incredibly complex (LabVIEW), or overly basic and rudimentary (DataStudio).
The result under this current system is one where proper research is often locked behind expensive paywalls, that greatly restrict the creative and intellectual freedoms of scientists. Access to the necessary tools to perform high-quality experiments is contingent on external funding, which comes with conditions imposed by the providers. This means that it can be exceedingly difficult for independent individuals or groups to even be able to afford access to tools necessary for research. This creates a model
of research-for-profit, and limited publicly funded projects. It is the position of the authors of this paper, that this imposes a severe impediment on the free conduct and advancement of science and research
for the betterment of the world. In addition to the difficulties found in acquiring access to this type of software, there is a specific lack of options for DAQ software tailored towards an undergraduate level of
learning, both from the perspective of the students who use the software, and from that of the professor, who needs to guide students through laboratory sessions using it.
While this is a wider-spread problem than a single type of software, the purpose of this project is to specifically address the problem of expensive, closed-source, proprietary DAQs. This project proposes
to develop and implement an Open-source, free alternative to the existing flagship software offerings on the market. It is not expected that this project will be initially capable of competing functionally with these software packages. Rather, it is simply the beginning of a small scale, user-focused software that keeps the spirit of research and  experimentation at its core. The software will start out as a specific
solution for one type of process, but it will be designed in such a way that it can be scaled and modified over time to encompass any number of different processes and experiments. This software will be built
by students, for use by students in laboratories and academia, using perspectives and insight gained through use of other similar software. Emphasis will be placed on ease of use and functionality that
facilitates learning.
# Project Description And Outline:
The entire scope of the project encompasses a general data-pipeline process, starting with the recording of raw data signals using sensors, and ending with the the manipulation and analysis of that data in
a specialised DAQ with a Graphical User Interface (GUI).
The Project can be broken down into the following phases:
1. Reading of analog signal from physical process and conversion from analog to digital (ADC)
2. Transfer of digital information to host device (End-User Computer)
3. Internal access and porting of data into DAQ Application.
4. End User manipulation of data in a GUI.
# Toolchain:
The following tools, languages, and devices will be used:
## C++ for the main application
C++ is being used for this application for a few main reasons. First, it is an industry standard for high-performance applications that handle large amounts of data in real-time. As a purely compiled language, the project can achieve maximum cross-platform compatibility – If it compiles,
then it runs – and every major Operating System will compile C++ natively.
## wxWidgets for the GUI to maximize cross-platform portability and achieve a standard, native feel to the application
This library for C++ GUI creation is ideal for creating a cross-platform GUI that adopts the native feel of applications in each OS. It is lighter weight than QT, and has no licensing restrictions.
## Arduino for converting analog signals to digital, and porting data to a Host Device
Using an Arduino allows the first two phases of the project to become trivial. Arduino devices come pre-configured to communicate via USB to a Host Device using Serial or I2c protocols. This
allows developers to focus simply on reading the data stream from the connected port, and manipulate it within the application.
## CMake build automation tool to make project compilation IDE and OS agnostic 
Using a specific IDE from the beginning can create problems when users or 3rd parties attempt to compile source code from a project file, because each IDE creates a proprietary signature that makes it incompatible
with others. Using CMake allows developers to create project files for all of the main IDE options, while retaining the ability to simply compile from a command line using the Makefile that CMake
generates automatically for the project.
## SQLite 
will be included in the project as a way to store application configurations, project files, and structured data. In this case, security is not a concern by the developers, so the light weight, single-file cross-platform features of the SQLite database make it an ideal candidate.
# Phase 1: Signal Processing
## Discussion:
In Phase 1 of this project, the focus will be placed on obtaining raw signals from analog sensors connected to a singular piece of lab equipment. In the future, additional equipment compatibility will be
added, but for now, the scope will be restricted to allow for faster development. This signal will use an Arduino, or similar microcontroller-based device to read the raw signal input(s) from the sensor(s). The
main benefit of using an Arduino is the ease with which data can be obtained and used. Arduino framework allows for a very easy process of polling sensors, as well as a host of internal methods that can be
used to manipulate that input. Additionally, such devices are already configured to communicate over Universal Serial Bus (USB) with a host device. Such devices come installed with all necessary drivers and
firmware necessary to automatically establish a handshake with the host device and send data directly through an open port. This will be very convenient when it comes time to transfer data to the host device.
Eventually, after the software is up and running, this phase will be re-evaluated, and it is possible that an effort will be made to come up with a custom sensor ADC USB device that has its own drivers
and can communicate with a host device. This will be mostly to showcase embedded C programming capabilities of the developers, as well as gain some freedom to define custom communication protocols
or data packaging.
## Implementation:
While the specific piece of lab equipment is not yet known, something such as a pressure sensor will serve as a good example of how this process will be accomplished. 
First, an Arduino or ESP32 board will be configured with a simple polling loop to obtain sensor input signals. Such a sensor will output a constant stream of values in milliamperes or volts between a certain
range to represent a measure of some unit. From this point, there are two options:
1. directly transmit raw signal data to host device via Phase 2.
2. perform direct manipulation of the input signal directly within the microcontroller.
It is likely that the first option will be selected. In order to enhance project scalability, flexibility, and modularity, raw data will be sent directly to the main application, where its interpretation will be
implemented. This is seen as the ideal option, due to the fact that the interpretation of raw signal is not standardized across sensors. Leaving the interpretation as a responsibility of a main-program module
allows for dynamic signal processing that can be applied to a range of different sensors and applications that can be directly manipulated or specified via a GUI within the application.
One consideration to make for the use of Arduino boards, is that analog-to-digital data conversion has a resolution limit of 10 bits. In other words, analog values are mapped to binary values between 0 - 1023. This imposes a limitation on the precision with which processes can be measured, but for the scope of this project, it will not present a significant obstacle.
# Phase 2: Data Transfer To Host Device
## Discussion:
Once the data is available within the Arduino or micro-controller, it will be transmitted to an available host device port. Once again, a benefit of using something pre-configured, is that the device is already able to communicate via the same port used to program it. All that is left is agree on and select a protocol, and to transmit to the Host Device port using it.
## Implementation:
Before anything else, an interface will be designed and implemented for the structured transmission of sensor data to the host device. In applications that use multiple sensors, it will be desirable to maintain
distinction between each individual sensor and its data. Additionally, it may be beneficial to consider a more structured approach for the actual streaming of data to increase efficiency and to facilitate more
intelligent communication between the sensor package and the host device. It stands to reason that it would be a waste of resources to have the sensors operating and streaming data at high rates, even when
not required by the DAQ. To avoid this, it is possible to implement slower baud rate, and to establish a "data-on-demand" relationship between host device and sensor package.
# Phase 3: Host Application Access Of Data
## Discussion:
The data from the Arduino will be present and transmitted as a stream to the connected host device port.
It will then be the responsibility of the main application to open and read data from that port. Depending on what communication protocol is used, an appropriate library will be used to intercept and interpret
the stream of data from the port.
## Implementation:
While the exact details of this are not yet known, this will be accomplished first by defining a class that handles the interfacing with the sensor package. This will be the interface that enforces the previously mention data structuring standard, and any communications between the host device and sensor package. Any data to be used in the DAQ must be passed through this interface class, which will ensure the
efficient packaging and accessibility of the data.
# Phase 4: Manipulation and analysis of data through Application GUI
## Discussion:
At this final stage, the data is ported and usable by the main program. End Users are able to map sensor inputs to outputs and/or measurements (temperature, pressure, distance, etc...). The application will allow users to perform operations and analysis on data via the GUI.
## Implementation:
This is where the majority of time will be spent on the project. This phase will be broken up into the following main components:
### Raw data conversion: 
Once the data is packaged and parsed for use in the DAQ, the next thing that will need to happen is signal processing. This means that a class will need to be defined whose responsibility is the mapping of raw sensor data into a meaningful measurement, for example
temperature or pressure. This will be accomplished by incorporating a functionality to plot data points from a sensor specification sheet and generate a function that can map the raw data to the
appropriate measurement value.
### Application configuration options: 
in this portion, using SQLite, application configuration settings can be changed, saved, and loaded. This includes all GUI-related aspects of the access and manipulation of these parameters as desired.
### Modular experiment/project class: 
It will be helpful to define a class that represents a single "experiment" in which all available DAQ functionalities may be selected and used as appropriate. In other words, users should be able to create an experiment or project, and define which sensors to use, how to interpret them, display the data in real-time, record the data over time, and save the data in a structured format (.csv for example). This way, there can be a general template for how an experiment appears and behaves graphically for users, while allowing parameters to be set, saved, and loaded according to the wishes of the end user. The idea is that once a certain type of experiment is configured and set up, it can be loaded and reused. This will also learn on SQLite to achieve storage and access of project files.
### Additional Functionalities within an experiment: 
In addition to being able to view, record, and save data, it will be helpful to perform various operations on the data, such as fitting curves,
extracting key data, and performing statistical analyses. This portion is going to be the last to be achieved, as it is more in line with items from the future wish-list.
# General to-dos
1. Learn how to create GUI applications in C++ using wxWidgets
2. Conceptualize and model a modular experiment-based data-structure (in the application, an "experiment" is created or opened, and each experiment file has an arbitrary number of general parameters that can be adjusted and tweaked.)
• I/O parameters
• Graphical options (using imported .png images etc..)
3. Learn how to transfer data from an Arduino to a host device and read it in a C++ program.
4. Create a C++ class that can automatically generate a mapping function for analog signals to an appropriate unit of measurement. There should be options for entering explicit equations, or using a graphical reference (image) to identify a curve and automatically perform a fit to the sensor data.
5. Create a graphical sketch of all main program components, windows, parameters, and functionalities.
6. Write setup guides for both hardware and software.
7. Create API documentation for software.
8. Make troubleshooting guides.
9. Open issues and features requests for students/professors/users feedback.
# Future Potential Additions
1. Interactive 3D Visualization: Add a 3D model of the machine that users can rotate, zoom, and explore. Clicking on each part would display its name, description and role in the experiment.
2. Smart Image Capture: Integrate a camera to automatically capture pictures of experimental results, then polish them using code that can adjust color and light, or even AI for a clearer and more
professional visuals.
