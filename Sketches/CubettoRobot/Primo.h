// Primo version number
#define PRIMO_CUBETTO_PLAYSET_VERSION "1.0"

////////////////////////////////////////////////////////////////////////////////

// Command codes
#define PRIMO_COMMAND_NONE     0
#define PRIMO_COMMAND_RIGHT    1
#define PRIMO_COMMAND_LEFT     2
#define PRIMO_COMMAND_FORWARD  4
#define PRIMO_COMMAND_FUNCTION 5

////////////////////////////////////////////////////////////////////////////////

// This 32-bit value is the identifier for ANY Cubetto Playset.  It is inserted
// into every radio packet sent by ANY Interface.  A check is made on every
// radio packet, received by ANY Cubetto Robot, that this value is present.
#define PRIMO_INTERFACE_ID 0xDE1D8758UL

// This 32-bit value is the identifier for ANY Cubetto Playset.  It is inserted
// into every radio packet sent by ANY Cubetto Robot.  A check is made on every
// radio packet, received by ANY Interface, that this value is present.
#define PRIMO_CUBETTO_ROBOT_ID 0xDE1D8759UL

// Maximum number of instructions that can be sent in a message
#define PRIMO_MAX_MAIN_INSTRUCTIONS     12
#define PRIMO_MAX_FUNCTION_INSTRUCTIONS 4

typedef struct commandsMessage
{
  uint32_t senderId;
  uint32_t sessionId;
  uint8_t mainInstructions[PRIMO_MAX_MAIN_INSTRUCTIONS];
  uint8_t functionInstructions[PRIMO_MAX_FUNCTION_INSTRUCTIONS];
  uint8_t checksum;
}
CommandsMessage;

typedef struct ackMessage
{
  uint32_t senderId;
  uint32_t sessionId;
  uint32_t messageCounter;
}
AckMessage;

uint32_t sessionId = 0UL;
uint32_t ackMessageCount = 0UL;

// Communication pipes addresses
//uint8_t PRIMO_INTERFACE2CUBETTO_PIPE_ADDRESS[5] = { 0xCC,0xCE,0xCC,0xCE,0xCC };
//uint8_t PRIMO_CUBETTO2INTERFACE_PIPE_ADDRESS[5] = { 0xCE,0xCC,0xCE,0xCC,0xCE };
uint8_t PRIMO_INTERFACE2CUBETTO_PIPE_ADDRESS[5] = { 0xE8,0xE8,0xF0,0xF0,0xE1 };
uint8_t PRIMO_CUBETTO2INTERFACE_PIPE_ADDRESS[5] = { 0xE8,0xE8,0xF0,0xF0,0xE2 };

