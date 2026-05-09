#include "mbed.h"

// ------------------ LED OUTPUTS (active-LOW: 0 = ON, 1 = OFF) ------------------

DigitalOut conveyorGreen(D0);   // Conveyor moving (GREEN)
DigitalOut conveyorRed(D1);     // Conveyor stopped (RED)
DigitalOut nozzleGreen(D2);     // Nozzle filling (GREEN)
DigitalOut nozzleRed(D3);       // Nozzle closed (RED)

// DIP SWITCH INPUTS (pull-down, ON = 1) 
// Box count (4 bits)
DigitalIn box0(D4);   // LSB
DigitalIn box1(D5);
DigitalIn box2(D6);
DigitalIn box3(D7);   // MSB

// Pieces per box (4 bits)
DigitalIn piece0(D8);   // LSB
DigitalIn piece1(D9);
DigitalIn piece2(D10);
DigitalIn piece3(D11);  // MSB

// BUTTON INPUTS (on-board, active-LOW) 
DigitalIn startButton(SW1);   // START
DigitalIn stopButton(SW3);        // STOP / EMERGENCY

// STATE DEFINITIONS 

#define STATE_IDLE       0
#define STATE_MOVE_BOX   1
#define STATE_FILL_BOX   2
#define STATE_DONE       3

// ------------------ GLOBAL VARIABLES ------------------

int state = STATE_IDLE;
int totalBoxes = 0;
int boxesRemaining = 0;
int piecesPerBox = 0;

// ------------------ HELPER FUNCTIONS ------------------

// Turn OFF all LEDs (active-LOW)
void all_off()
{
    conveyorGreen = 1;
    conveyorRed   = 1;
    nozzleGreen   = 1;
    nozzleRed     = 1;
}

// Make sure conveyor is moving, nozzle closed
void set_conveyor_moving()
{
    all_off();
    conveyorGreen = 0;   // ON
    nozzleRed     = 0;   // nozzle closed
}

// Conveyor stopped, nozzle closed (idle/stop)
void set_all_stopped()
{
    all_off();
    conveyorRed = 0;    // conveyor stopped
    nozzleRed   = 0;    // nozzle closed
}

// Conveyor stopped, nozzle filling
void set_nozzle_filling()
{
    all_off();
    conveyorRed   = 0;  // conveyor stopped
    nozzleGreen   = 0;  // nozzle filling
}

// Read 4-bit box count from DIP
int read_boxes()
{
    int value = 0;
    value |= (box0.read() << 0);
    value |= (box1.read() << 1);
    value |= (box2.read() << 2);
    value |= (box3.read() << 3);
    return value;
}

// Read 4-bit pieces per box from DIP
int read_pieces()
{
    int value = 0;
    value |= (piece0.read() << 0);
    value |= (piece1.read() << 1);
    value |= (piece2.read() << 2);
    value |= (piece3.read() << 3);
    return value;
}

// Convenience: check if STOP is pressed (active-LOW)
int stop_pressed()
{
    return (stopButton.read() == 0);
}

// Convenience: check if START is pressed (active-LOW)
int start_pressed()
{
    return (startButton.read() == 0);
}

// ------------------ STATE THE FUNCTIONS ------------------

// IDLE: wait for START, show both red LEDs
void state_idle()
{
    set_all_stopped();

    // Wait here until START is pressed, check if stop is pressed
    while (1)
    {
        if (stop_pressed())
        {
            // stay in idle, already all stopped
            set_all_stopped();
        }

        if (start_pressed())
        {
            // Read configuration from DIP switches
            totalBoxes    = read_boxes();
            piecesPerBox  = read_pieces();
            boxesRemaining = totalBoxes;

            // Simple sanity check: if 0 boxes, stay idle
            if (boxesRemaining > 0 && piecesPerBox > 0)
            {
                state = STATE_MOVE_BOX;
                return;
            }
            // If invalid config (0), just stay idle
        }

        thread_sleep_for(50);
    }
}

// MOVE_BOX: move one box to nozzle position (5 seconds)
// Conveyor green ON, nozzle closed; check STOP frequently
void state_move_box()
{
    set_conveyor_moving();

    int elapsed = 0;
    int totalTime = 5000;  // 5 seconds

    while (elapsed < totalTime)
    {
        if (stop_pressed())
        {
            // Emergency stop
            state = STATE_IDLE;
            set_all_stopped();
            return;
        }

        thread_sleep_for(100);
        elapsed += 100;
    }

    // After 5s, box is under nozzle
    state = STATE_FILL_BOX;
}

// FILL_BOX: nozzle dispenses piecesPerBox pieces, 1s per piece
// Conveyor stopped (red), nozzle filling (green)
void state_fill_box()
{
    int i, t;

    for (i = 0; i < piecesPerBox; i++)
    {
        set_nozzle_filling();

        // Dispense one piece = 1 second
        t = 0;
        while (t < 1000)
        {
            if (stop_pressed())
            {
                // Emergency stop
                state = STATE_IDLE;
                set_all_stopped();
                return;
            }

            thread_sleep_for(100);
            t += 100;
        }
    }

    // Box filled
    boxesRemaining--;

    // Turn nozzle off, conveyor stopped
    set_all_stopped();

    if (boxesRemaining > 0)
    {
        state = STATE_MOVE_BOX;   // next box
    }
    else
    {
        state = STATE_DONE;
    }
}

// DONE: all boxes filled; show both red LEDs, then wait for START again
void state_done()
{
    set_all_stopped();

    // Wait until START is pressed again to restart with new DIP config
    while (1)
    {
        if (stop_pressed())
        {
            // Still all stopped, nothing else to do
            set_all_stopped();
        }

        if (start_pressed())
        {
            // Go back to idle to read new configuration
            state = STATE_IDLE;
            return;
        }

        thread_sleep_for(50);
    }
}

// ------------------ MAIN ------------------

int main()
{
    // Configure button pull-ups (active-LOW)
    startButton.mode(PullUp);
    stopButton.mode(PullUp);

    // Initial state
    state = STATE_IDLE;
    set_all_stopped();

    while (1)
    {
        switch (state)
        {
            case STATE_IDLE:
                state_idle();
                break;

            case STATE_MOVE_BOX:
                state_move_box();
                break;

            case STATE_FILL_BOX:
                state_fill_box();
                break;

            case STATE_DONE:
                state_done();
                break;

            default:
                state = STATE_IDLE;
                break;
        }
    }
}
