#include <main.hpp>

//#if WIN32
//INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
//#else
int main (int argc, char **argv)
//#endif // WIN32
{
	extern char *optarg; // Used for getopt()
	extern int optind;

	// Make a System65 instance
	System65 sys;

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

	// Make a render window
	sf::RenderWindow window(sf::VideoMode(896,464), "System65 Emulator", sf::Style::Close);
	window.setFramerateLimit(60);

	// Load the font
	sf::Texture fonttex;
	if (!fonttex.loadFromFile("font_82.png"))
		return 1;

	// Literally an array of characters
	// Create it, set them all to ' ' (space) and position them accordingly.
	sf::Sprite screenbuf[EMUSCREEN_WIDTH][EMUSCREEN_HEIGHT];
	for (int x = 0; x < EMUSCREEN_WIDTH; x++) {
		for (int y = 0; y < EMUSCREEN_HEIGHT; y++) {
			screenbuf[x][y].setTexture(fonttex);
			screenbuf[x][y].setTextureRect(sf::IntRect(0,0,8,16)); // left, top, width, height
			screenbuf[x][y].setPosition(sf::Vector2f((x*TEXCHAR_WIDTH),(y*TEXCHAR_HEIGHT)));
		}
	}

	// Create the frame
	DrawScreenFrame(screenbuf);

	// Create the static elements
	DrawLabels(screenbuf);

	// Initial draw of the processor status
	DrawStats(screenbuf,&sys);

	// Main loop
	while(window.isOpen()) {
		sf::Event event;

		// Step vm execution
		sys.Tick();

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				printf("[DEBUG] Window closed\n");
				window.close();
			}
		}

		// Draw the window buffer
		window.clear(sf::Color::Black);
		DrawStats(screenbuf,&sys); // Update the onscreen CPU state
		for (int x = 0; x < EMUSCREEN_WIDTH; x++) {
			for (int y = 0; y < EMUSCREEN_HEIGHT; y++)
					window.draw(screenbuf[x][y]);
		}
		window.display();
	}

	printf("Finished\n");
	return 0;
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
//	for (int i = 0b00000001; i < 256; i <<= 1)
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
	// TODO: Use a better algo to find the tex coords
	// Currently: character val is multiplied by 8 to get the X coordinate of a
	// single-row texture. We successively subtract the actual row size until
	// X is lower, and add a column to Y each time. Once X is lower than 128, we
	// should have the proper coordinates.
	unsigned int xpos, ypos = 0;
	unsigned char cr = (unsigned char)c;
	xpos = (unsigned)cr*8;
	//printf("[DEBUG] c = %c (%u), xpos = %u, final = ",c,(unsigned)c,xpos);
	if (xpos > 127) {
		while (xpos > 127) {
			xpos -= 128;
			ypos += 16;
		}
	}
	//printf("%u,%u (%u,%u)\n",xpos,ypos,xpos/8,ypos/16);
	screenbuf[x][y].setTextureRect(sf::IntRect(xpos,ypos,8,16));
}

void ShowHelp(void)
{
	printf("Available options: \n");
	// -b -s -i
	printf("-b filename: Loads a file into program memory\n");
	printf("-s 0xNN    : Sets the stack base to 0xNN00; default is 0x20\n");
	printf("-i 0xNNNN  : Sets the interrupt vector to 0xNNNN; default is 0xFFFE\n");
	printf("-h         : Shows this help text\n");
}
