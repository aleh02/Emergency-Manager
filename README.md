# 🚨 Emergency Management System
A multithreaded emergency dispatching system written in C (C11), implementing message queues, concurrency control, and digital-twin responders.  
Developed as part of *Laboratorio 2 – Università di Pisa*.

---

## 📌 Overview
This project simulates a real-time emergency management platform where emergencies are received, queued, prioritized, and assigned to available responders.

The system is composed of multiple modules (dispatcher, queue, responders, logger...) and relies on:
- **C11 threads (`thrd_t`)**
- **POSIX message queues**
- **Mutexes (`mtx_t`)**
- **Priority-based emergency scheduling**
- **Separate threads for responders returning from missions**

The goal is to reproduce realistic emergency handling: reception, prioritization, assignment, mission duration, and responder return.

---

## 🧩 Features

### ✔ Emergency Reception
Incoming emergencies are sent through a **POSIX message queue** and handled asynchronously by the system.

### ✔ Priority Queue
Emergencies are stored in a **priority queue**, ordered by:
1. Severity  
2. Timestamp (FIFO within same severity)

### ✔ Dispatcher Module
A dedicated dispatcher thread:
- Monitors the priority queue  
- Assigns emergencies to available responders (digital twins)  
- Spawns **mission threads** to simulate real-world timing  
- Logs every relevant event  


### ✔ Responders (Digital Twins)
Each responder:
- Can handle **one emergency at a time**
- Has a dedicated thread simulating mission duration
- Returns asynchronously, notifying the dispatcher of availability

### ✔ Logging System
Every event (enqueue, dequeue, assignment, completion…) is recorded through a centralized logger.

### ✔ Clean Resource Management
- Proper initialization and destruction of mutexes  
- Graceful shutdown  
- `dispatcher_destroy()` correctly releases resources

---

## 🏛 Architecture
+-------------------+ +----------------------+

| POSIX Message | -----> | Emergency Receiver |

| Queue | +----------------------+

| (incoming calls) | |

+-------------------+ v

+-------------------------+

| Priority Queue |

+-------------------------+

|

v

+-------------------------+

| Dispatcher |

| (thread + queue lock) |

+-------------------------+

| assigns emergencies to |

v available units

+------------------+ +------------------+

| Responder #1 | | Responder #2 |

| (thread, mission)| | (thread, mission)|

+------------------+ +------------------+


---

## 🗂 Project Structure

/src

├── dispatcher.c / dispatcher.h

├── emergency.c / emergency.h

├── queue.c / queue.h

├── responder.c / responder.h

├── logger.c / logger.h

├── utils.c / utils.h

├── main.c


- **dispatcher/** → core scheduling logic, mutex-protected queue, responder assignment  
- **queue/** → priority queue implementation  
- **responder/** → thread routines for emergency missions  
- **logger/** → thread-safe logging  
- **main.c** → initialization, configuration, startup and shutdown  

---

## ⚙️ Build & Run

### ### 🔧 Requirements
- GCC or Clang
- POSIX-compliant environment (Linux, macOS)
- Make

### ▶ Compile
```bash
make

./emergency_system

[INFO] Emergency received: ID=42, PRIORITY=3
[INFO] Added to priority queue
[INFO] Dispatcher assigning emergency 42 to Responder #2
[INFO] Responder #2 started mission (duration: 12s)
[INFO] Responder #2 returned from mission

```

---

## 🧠 Core Concepts Demonstrated

-Concurrency and synchronization

-Thread lifecycle management

-Message-passing via POSIX MQ

-Priority-based scheduling

-Realistic simulation of time-based processes

-Modular design and clean architecture

---

## 👤 Author

Alessandro Han

Computer Science, University of Pisa

LinkedIn: https://www.linkedin.com/in/alessandro-han-b87391223/
