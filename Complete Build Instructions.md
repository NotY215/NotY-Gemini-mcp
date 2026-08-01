## Complete Build Instructions

### Step 1: Build C++ Backend
```bash
cd F:\Own Apps\NotY-Gemini-mcp
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Step 2: Build Java UI
```bash
cd F:\Own Apps\NotY-Gemini-mcp
gradle build
```

### Step 3: Run the Application
```bash
gradle run
```

### Step 4: Create Distribution
```bash
gradle jar
# Copy the JAR file and the compiled DLL to same directory
# Copy geminicore.dll (from build/bin/Release/) to same folder as JAR
```

## Project Structure Summary

```
NotY-Gemini-mcp/
├── src/
│   ├── cpp/
│   │   ├── main.cpp
│   │   ├── web_server.cpp/h
│   │   ├── gemini_service.cpp/h
│   │   ├── vscode_manager.cpp/h
│   │   ├── encryption.cpp/h
│   │   ├── config_manager.cpp/h
│   │   ├── logger.cpp/h
│   │   └── jni_bridge.cpp/h
│   └── java/com/noty/geminimcp/
│       ├── MainApp.java
│       ├── MainWindow.java
│       ├── VSCodeSetupPanel.java
│       ├── ApiKeySetupPanel.java
│       ├── ServerControlPanel.java
│       ├── TermsDialog.java
│       ├── ToastNotification.java
│       ├── Theme.java
│       ├── NativeBridge.java
│       └── TrayManager.java
├── resources/
│   ├── icon.png
│   └── tray-icon.png
├── build.gradle
├── CMakeLists.txt
└── vcpkg.json
```