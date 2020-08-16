all: emulator

emulator: main.cpp Linker.cpp FileReader.cpp Processor.cpp Memory.cpp TerminalHandler.cpp TimerHandler.cpp
	g++ main.cpp Linker.cpp Processor.cpp FileReader.cpp Memory.cpp TerminalHandler.cpp TimerHandler.cpp -o emulator




