# Simple HTTP Web Server

## 📜 About the Project
This project was developed as part of the *Computer Networks* coursework, with the goal of implementing a **simple HTTP/1.1 web server** in C.  
While a complete web server can be quite complex, this simplified version focuses on the core functionality of serving static files and understanding the fundamentals of HTTP communication.

---

## 🎯 Features
- Accepts only **GET** requests.
- Serves content from a designated folder (`/www_root`) and its subfolders.
- Direct mapping between URI paths and file paths.
  - Example: `/img/img1.jpeg` → `/www_root/img/img1.jpeg`
- Directory requests automatically return `index.html` (if present).
- Returns HTTP **404 Not Found** if the requested file does not exist.
- Full compliance with **HTTP/1.1** protocol basics.
- Adds `Server` and `Date` headers to responses.
- Logs all requests to `stderr`.
- Compatible with any modern web browser (e.g., Chrome, Firefox).
- Loads the web page `favicon.ico` if has one

---

## 📂 Project Structure

- `httpd.h` # Framework header file
- `httpd.c` # Framework source file
- `main.c` # Main program
- `/www_root` # Directory that contains the web page files

 ## 🚀 How to Run

**CHANGE ON `main.c` THE PATH ON WWW_ROOT**

 ### Linux

```
 gcc src/*.c -o httpd && ./httpd
```
