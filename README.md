# Temporal Pathfinder 🚆

![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![Architecture](https://img.shields.io/badge/Architecture-Full--Stack-orange)
![License](https://img.shields.io/badge/License-MIT-green)

A full-stack, multi-modal public transit routing engine. This project features a high-performance C++ backend that runs a local web server and an interactive web frontend for visualizing the network and journey results. It uses the state-of-the-art Multi-Criteria RAPTOR algorithm to find optimal journeys through real-world GTFS data, balancing arrival times and the number of transfers.

---
## Screenshot

*(Replace this with your own screenshot of the final application in action!)*



---
## What This Project Really Does

This project is a **temporal pathfinder**. It solves the complex problem of navigating a public transit system. It looks at all the bus/train schedules and walking paths and answers the question: **"What are the best possible travel plans to get from A to B, leaving around a certain time?"**

> **The Core Problem:** Navigating a city with public transit isn't about finding the shortest line between two points. It's about catching a series of "moving platforms" (buses, trains) that are only available at specific moments. A path that is physically short might be incredibly slow if you just missed a bus and have to wait 30 minutes for the next one. A physically longer path on an express train might get you there much faster.

This application models and solves that exact problem. Here's how:

1.  **It Understands Real-World Schedules:** The engine consumes standard **GTFS (General Transit Feed Specification)** data from real transit agencies. It parses not just the stops and routes, but the complex timetables (`stop_times.txt`) and daily schedules (`calendar.txt`) to know exactly when every trip is running.

2.  **It Uses a Specialized Algorithm (RAPTOR):** Instead of a simple algorithm like Dijkstra's, this project uses the **Multi-Criteria RAPTOR** algorithm. RAPTOR is specifically designed for public transit. It works in "rounds," where each round corresponds to an additional transfer. This allows it to naturally find paths that are both fast and have a low number of transfers.

3.  **It Finds "Smart" Options, Not Just One:** The engine performs a **multi-criteria search**. It understands that the "best" path is subjective. Is it the one that arrives earliest, or the one with fewer stressful transfers? Instead of choosing for you, it calculates a profile of **non-dominated journeys** (a Pareto front). This means it gives you a list of all the "smartest" trade-offs, automatically filtering out any option that is strictly worse than another.

4.  **It Integrates Multiple Modes of Transport:** The algorithm seamlessly combines the scheduled vehicle network with the "always available" walking network (`transfers.txt`), allowing it to find clever routes that might involve, for example, taking a bus, walking a few blocks, and then catching a different train line.

---
## ✨ Key Features

* **Advanced Routing Engine:** Implements the Multi-Criteria RAPTOR algorithm.
* **Multi-Modal Journeys:** Integrates scheduled vehicle trips with walking transfers.
* **Multi-Criteria Search:** Balances earliest arrival time against the number of transfers to provide a profile of optimal choices.
* **Real GTFS Data Support:** Parses standard GTFS feeds, including daily schedules from `calendar.txt`.
* **Full-Stack Architecture:** A decoupled system with a C++ backend web server and a JavaScript frontend.
* **Interactive UI:** A user-friendly web interface with autocomplete stop search and an interactive SVG graph visualizer.

---
## 💻 Technology Stack

* **Backend:**
    * **C++17:** For the core high-performance routing engine.
    * **cpp-httplib:** A lightweight, header-only C++ library for the web server.
* **Frontend:**
    * **HTML5, CSS3, JavaScript (ES6+):** For the user interface and interactivity.
    * **SVG (Scalable Vector Graphics):** To render the abstract graph of the transit network.
* **Data Format:**
    * **GTFS:** For transit data.
    * **JSON:** For API communication between the backend and frontend.

---
## 🚀 Setup and Running

Follow these steps to set up and run the project.

### 1. Prerequisites
* A C++ compiler that supports C++17 (like a modern version of MinGW/GCC).
* Code::Blocks IDE (or another C++ IDE).
* A modern web browser.

### 2. Backend Setup
1.  **Project Structure:** Organize your C++ files as a multi-file project: `main.cpp`, `Raptor.h`, `Raptor.cpp`, `DataTypes.h`.
2.  **Add Libraries:** Place the `httplib.h` header file in your main project directory.
3.  **Configure Linker (Windows/MinGW):** In Code::Blocks, go to `Settings -> Compiler -> Linker settings`. In "Other linker options", add the flag: `-lws2_32`.
4.  **Add Source Files:** Ensure `main.cpp` and `Raptor.cpp` are both added to your Code::Blocks project sources.

### 3. Data Setup
1.  Download a GTFS feed and unzip it.
2.  Create a `bin/Debug/` subfolder in your project directory.
3.  Copy the GTFS files (`stops.txt`, `routes.txt`, `trips.txt`, `stop_times.txt`, `calendar.txt`, and an optional `transfers.txt`) into the `bin/Debug/` folder.

### 4. Frontend Setup
1.  Copy the frontend files (`index.html`, `style.css`, `script.js`) into the same `bin/Debug/` folder.

### 5. Compile and Run
1.  In Code::Blocks, press **F9** (or `Build -> Build and run`).
2.  A terminal window will open with the message: `Server starting on http://localhost:8080`.
3.  Open your web browser and navigate to **`http://localhost:8080`**.

---
## API Documentation

The C++ server exposes the following API endpoints:

* **`GET /api/stops`**
    * Returns a JSON array of all stops, including their `id`, `name`, `lat`, and `lon`. Used to populate the search boxes.
* **`GET /api/network`**
    * Returns a JSON array of all connections (vehicle and walking) in the network, like `[{"from":1,"to":2}, ...]`. Used to draw the base graph.
* **`GET /api/route`**
    * **Parameters:** `from` (stop_id), `to` (stop_id), `time` (HH:MM:SS).
    * Executes the RAPTOR algorithm and returns a rich JSON object containing a profile of optimal journeys, including the detailed step-by-step path for each journey.
