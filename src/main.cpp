#include <main.hpp>

#if defined(_MSC_VER) && !defined(_DEBUG)
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main (int argc, char **argv)
#endif //_MSC_VER && !_DEBUG
{
	extern char *optarg; // Used for getopt()
	extern int optind;

	// Acquire the lock to keep the VM from running off
	mutMachineState.lock();

	// Make a thread for the system
	std::thread SystemThread(SystemExec);

	// TODO: Make file loading more C++-ey
#ifndef _MSC_VER
	int c;
	FILE *progfile = NULL;
	while ((c = getopt(argc,argv,"b:s:i:h")) != -1) {
		switch (c) {
		case 'b':
			// Load a program file into memory
			progfile = fopen(optarg,"rb");
			if (progfile != NULL) {
				printf("Loading file %s\n",optarg);
				sys.LoadProgram(progfile);
				fclose(progfile);
			} else {
				printf("Could not open %s\n",optarg);
				exit(1);
			}
			break;
		case 's':
			sys.SetStackBasePage((uint8_t)(atoi(optarg)));
			// Set the stack base
			break;
		case 'i':
			// Set the interrupt vector
			sys.SetInterruptVector((uint16_t)(atoi(optarg)));
			break;
		case 'h':
			// Show the usage text
			ShowHelp();
			exit(0);
		default:
			printf("Unrecognized option: %c\n",c);
			ShowHelp();
			exit(1);
		}
	}
#else //_MSC_VER
	// TODO: implement arbitrary file loading
	FILE *progfile = NULL;
	char *filename = "6502_functional_test.bin";
	progfile = fopen(filename,"rb");
	if (progfile != NULL) {
		printf("Loading file %s...\n",filename);
		sys.LoadProgram(progfile);
		fclose(progfile);
	} else {
		printf("Could not open %s\n",filename);
		exit(1);
	}
#endif //_MSC_VER

	// Make a render window
	// should be 896x348
	sf::RenderWindow window(sf::VideoMode(TEXCHAR_WIDTH*EMUSCREEN_WIDTH,TEXCHAR_HEIGHT*EMUSCREEN_HEIGHT), "System65 Emulator", sf::Style::Close);
	window.setFramerateLimit(60);

	// Load the font
	sf::Texture fonttex;
#ifdef _DEBUG
	printf("Max texture size: %i\n",fonttex.getMaximumSize());
#endif // _DEBUG
	// This will probably never happen, but let's handle it anyway.
	if (fonttex.getMaximumSize() < 256) {
		printf("Error: Your hardware does not support 256x256 textures.\n");
#ifdef WIN32
		MessageBox(NULL, L"Your hardware does not support 256x256 textures. This emulator cannot run.", L"Error", (MB_OK|MB_ICONERROR));
#endif // WIN32
		exit(1);
	}

	if (!fonttex.loadFromFile("font_82.bmp"))
		return 1;

	// Literally an array of characters
	// Create it, set them all to ' ' (space) and position them accordingly.
	sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT];
	for (int x = 0; x < EMUSCREEN_WIDTH; x++) {
		for (int y = 0; y < EMUSCREEN_HEIGHT; y++) {
			screenbuf[x][y].setTexture(fonttex);
			screenbuf[x][y].setTextureRect(sf::IntRect(0,0,TEXCHAR_WIDTH,TEXCHAR_HEIGHT)); // left, top, width, height
			screenbuf[x][y].setPosition(sf::Vector2f(((float)x*TEXCHAR_WIDTH),((float)y*TEXCHAR_HEIGHT)));
		}
	}

	// Create the frame
	DrawScreenFrame(screenbuf);

	// Create the static elements
	DrawLabels(screenbuf);

	// Initial draw of the processor status
	DrawStats(screenbuf,&sys);

	window.setVerticalSyncEnabled(true);

	// Unlock the VM to let it run
	mutMachineState.unlock();

	// Main loop
	while(window.isOpen()) {
		sf::Event event;

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				printf("[DEBUG] Window closed\n");
				window.close();
			}
		}

		// Draw the window buffer
		window.clear(sf::Color::Black);
		mutMachineState.lock();
		DrawStats(screenbuf,&sys); // Update the onscreen CPU state
		mutMachineState.unlock();
		for (int x = 0; x < EMUSCREEN_WIDTH; x++) {
			for (int y = 0; y < EMUSCREEN_HEIGHT; y++)
					window.draw(screenbuf[x][y]);
		}
		window.display();
	}

	printf("Finished\n");
	bStopExec = true;
	SystemThread.join();
	exit(0);
}

void SystemExec(void) {
	for (;;) {
		if (bStopExec)
			return;

		if (mutMachineState.try_lock()) {
			sys.Tick();
			mutMachineState.unlock();
		}
	}
}

void DrawScreenFrame(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT])
{
    // Draw the corner pieces
	DrawChar(screenbuf,(char)0xc9,0,                0                 ); // top left
	DrawChar(screenbuf,(char)0xc8,0,                EMUSCREEN_HEIGHT-1); // bottom left
	DrawChar(screenbuf,(char)0xbb,EMUSCREEN_WIDTH-1,0                 ); // top right
	DrawChar(screenbuf,(char)0xbc,EMUSCREEN_WIDTH-1,EMUSCREEN_HEIGHT-1); // bottom right

	// Draw the outermost borders
	for (int i = 1; i < EMUSCREEN_WIDTH-1; i++) { // Top/Bottom
		// should be 0xc4?
		DrawChar(screenbuf,(char)0xcd,i,0);
		DrawChar(screenbuf,(char)0xcd,i,EMUSCREEN_HEIGHT-1);
	}
	for (int i = 1; i < EMUSCREEN_HEIGHT-1; i++) {// Left/Right
		DrawChar(screenbuf,(char)0xba,0,i);
		DrawChar(screenbuf,(char)0xba,EMUSCREEN_WIDTH-1,i);
	}

	// Draw the status bar border
	DrawChar(screenbuf,(char)0xc7,0  ,26); // Left connector
	DrawChar(screenbuf,(char)0xb6,111,26); // Right connectorbt full
	for (int i = 1; i < EMUSCREEN_WIDTH-1; i++) // Border
		DrawChar((screenbuf),(char)0xc4,i,26);

	// Draw the right-hand monitor border
	DrawChar(screenbuf,(char)0xd1,81,0); // Top connector
	DrawChar(screenbuf,(char)0xc1,81,26); // Bottom connector
	for (int i = 1; i < EMUSCREEN_HEIGHT-3; i++) // Border
		DrawChar(screenbuf,(char)0xb3,81,i);

	// Draw the status/disasm border
	DrawChar(screenbuf,(char)0xc3,81,3); // Left connector
	DrawChar(screenbuf,(char)0xb6,111,3); // Right connector
	for (int i = 82; i < EMUSCREEN_WIDTH-1; i++) // Border
		DrawChar(screenbuf,(char)0xc4,i,3);
}

void DrawLabels(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT])
{
	DrawString(screenbuf,"NV-BDIZC  A>   X>   PC>",83,1);
	DrawString(screenbuf,"S>   Y>",93,2);
}

void DrawStats(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT], System65 *sys)
{
	// "But ccfreak2k," you say, "sprintf() is unsafe!"
	// "You are correct," I give in reply, "but witness that our data is neatly
	// "formed and easily constrained. An attacker must already be able to write
	// "to our memory in order to write to our memory with this!"
	char str[9] = {};
	sprintf(str,"%.2X",sys->GetRegister_A());
	DrawString(screenbuf,str,95,1);
	sprintf(str,"%.2X",sys->GetRegister_X());
	DrawString(screenbuf,str,100,1);
	sprintf(str,"%.2X",sys->GetRegister_Y());
	DrawString(screenbuf,str,100,2);
	sprintf(str,"%.2X",sys->GetRegister_S());
	DrawString(screenbuf,str,95,2);
	sprintf(str,"%.4X",sys->GetRegister_PC());
	DrawString(screenbuf,str,106,1);
	uint8_t f = sys->GetRegister_P();
	char *p = str;
//	for (int i = 0x01; i < 256; i <<= 1)
//		strcat(str,((f & i) == i) ? "1" : "0");
	for (int i = 0x80; i > 0; i >>= 1)
		*p++ = (f & i) ? '1' : '0';
	DrawString(screenbuf,str,83,2);
}

void DrawString(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT], const char *str, unsigned int x, unsigned int y)
{
	while (*str) {
		if (x >= EMUSCREEN_WIDTH) {
			x = 0;
			y++;
		}
		DrawChar(screenbuf,*str++,x++,y);
	}
}

void DrawChar(sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT], char c, unsigned int x, unsigned int y)
{
	unsigned char uc = (unsigned char)c;
	unsigned xpos = (uc % TEXCHAR_CELLS) * TEXCHAR_WIDTH;
	unsigned ypos = (uc / TEXCHAR_CELLS) * TEXCHAR_HEIGHT;
	//printf("%u,%u (%u,%u), c %c\n",xpos,ypos,xpos/TEXCHAR_WIDTH,ypos/TEXCHAR_HEIGHT,c);
	screenbuf[x][y].setTextureRect(sf::IntRect(xpos,ypos,TEXCHAR_WIDTH,TEXCHAR_HEIGHT));
}

void ShowHelp(void)
{
	printf("Available options: \n");
	// -b -s -i
	printf("-b filename: Loads a file into program memory\n");
	printf("-s 0xNN    : Sets the stack base to 0xNN00; default is 0x01\n");
	printf("-i 0xNNNN  : Sets the interrupt vector to 0xNNNN; default is 0xFFFE\n");
	printf("-h         : Shows this help text\n");
}
