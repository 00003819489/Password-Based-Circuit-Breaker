#include <reg51.h>

#define LCD P1

// LCD control pins
sbit RS = P2^0;
sbit EN = P2^2;

// Relay output
sbit RELAY = P2^1;

// Stored password
char password[4] = {'1','2','3','4'};
char entered[4];

// Keypad layout
char keypad_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// Attempt counter
int attempts = 0;

// ---------------- Delay Functions ----------------
void delay()
{
    unsigned int i;
    for(i=0;i<5000;i++);
}

void long_delay()
{
    unsigned int i,j;
    for(i=0;i<2000;i++)
    for(j=0;j<127;j++);
}

// ---------------- LCD Functions ----------------
void lcd_cmd(unsigned char cmd)
{
    LCD = cmd;
    RS = 0;
    EN = 1;
    delay();
    EN = 0;
}

void lcd_data(unsigned char dat)
{
    LCD = dat;
    RS = 1;
    EN = 1;
    delay();
    EN = 0;
}

void lcd_string(char *str)
{
    while(*str)
        lcd_data(*str++);
}

void lcd_init()
{
    lcd_cmd(0x38);   // 8-bit mode
    lcd_cmd(0x0C);   // Display ON
    lcd_cmd(0x01);   // Clear display
    lcd_cmd(0x06);   // Cursor increment
}

// ---------------- Keypad Function ----------------
char keypad()
{
    char row,col;

    while(1)
    {
        P3 = 0xF0;

        if((P3 & 0xF0) != 0xF0)
        {
            delay();

            for(row=0; row<4; row++)
            {
                P3 = ~(1<<row);

                for(col=0; col<4; col++)
                {
                    if(!(P3 & (0x10<<col)))
                    {
                        while(!(P3 & (0x10<<col)));
                        return keypad_map[row][col];
                    }
                }
            }
        }
    }
}

// ---------------- Main Program ----------------
void main()
{
    int i;

    RELAY = 0;  // Initially OFF

    lcd_init();
    lcd_string("ENTER PASSWORD");

    while(1)
    {
        lcd_cmd(0xC0);

        // Take 4-digit input
        for(i=0;i<4;i++)
        {
            entered[i] = keypad();
            lcd_data('*');
        }

        // Compare password
        for(i=0;i<4;i++)
        {
            if(entered[i] != password[i])
            {
                attempts++;

                lcd_cmd(0x01);
                lcd_string("ACCESS DENIED");

                long_delay();
                long_delay();

                // Lock after 3 wrong attempts
                if(attempts >= 3)
                {
                    lcd_cmd(0x01);
                    lcd_string("SYSTEM LOCKED");

                    long_delay();
                    long_delay();
                    long_delay();

                    attempts = 0;
                }

                lcd_cmd(0x01);
                lcd_string("ENTER PASSWORD");

                break;
            }
        }

        // Correct password
        if(i == 4)
        {
            attempts = 0;

            lcd_cmd(0x01);
            lcd_string("ACCESS GRANTED");

            RELAY = 1;   // Turn ON

            long_delay();
            long_delay();

            RELAY = 0;   // Turn OFF

            lcd_cmd(0x01);
            lcd_string("ENTER PASSWORD");
        }
    }
}