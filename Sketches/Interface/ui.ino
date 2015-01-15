void writeLed (uint8_t ledNumber, uint8_t ledStatus)
{
  // ledStatus = PRIMO_LED_ON to turn on, PRIMO_LED_OFF for off
  
  //debug_printf("LED %x - %x\r\n",led_number,onoff);

  switch (ledNumber)
  {
    case 1:
      gpioExp1.digitalWrite(3, ledStatus);
      break;

    case 2:
      gpioExp1.digitalWrite(7, ledStatus);
      break;

    case 3:
      gpioExp1.digitalWrite(11, ledStatus);
      break;

    case 4:
      gpioExp1.digitalWrite(15, ledStatus);
      break;

    case 5:
      gpioExp2.digitalWrite(3, ledStatus);
      break;

    case 6:
      gpioExp2.digitalWrite(7, ledStatus);
      break;

    case 7:
      gpioExp2.digitalWrite(11, ledStatus);
      break;

    case 8:
      gpioExp2.digitalWrite(15, ledStatus);
      break;

    case 9:
      gpioExp3.digitalWrite(3, ledStatus);
      break;

    case 10:
      gpioExp3.digitalWrite(7, ledStatus);
      break;

    case 11:
      gpioExp3.digitalWrite(11, ledStatus);
      break;

    case 12:
      gpioExp3.digitalWrite(15, ledStatus);
      break;

    case 13:
      gpioExp4.digitalWrite(3, ledStatus);
      break;

    case 14:
      gpioExp4.digitalWrite(7, ledStatus);
      break;

    case 15:
      gpioExp4.digitalWrite(11, ledStatus);
      break;

    case 16:
      gpioExp4.digitalWrite(15, ledStatus);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void switchAllLedsOn()
{
  for (int i = 0; i < 16;)
    writeLed(++i, PRIMO_LED_ON);
}

////////////////////////////////////////////////////////////////////////////////

void switchAllLedsOff()
{
  for (int i = 0; i < 16;)
    writeLed(++i, PRIMO_LED_OFF);
}

////////////////////////////////////////////////////////////////////////////////

char check_button (char button_to_check)
{
  // takes a button number (1-12 for standard buttons, 13-16 for the function buttons)
  
  // returns the function for that button - PRIMO_MAGNET_FORWARD, PRIMO_MAGNET_LEFT, PRIMO_MAGNET_RIGHT, PRIMO_MAGNET_FUNCTION or PRIMO_MAGNET_NONE
  
  // simplest way to process is in a case - only 16 possibilities!
  int hall_response;
  char button, t_button;
  char ret_val;
  char invert_magnet; // used for line 2, as the magnets are inserted 'upside down'
  char twisted; // one of the sensors is twisted - swap a couple of bits

  // debug_printf("Checking button\r\n");

  invert_magnet = 0;
  twisted = 0;

  switch (button_to_check)
  {
    case 1:
      hall_response = gpioExp1.readPort();
      button = (hall_response & 0x0F);
      break;

    case 2:
      hall_response = gpioExp1.readPort();
      button = (hall_response >> 4) & 0x0F;
      break;

    case 3:
      hall_response = gpioExp1.readPort();
      button = (hall_response >> 8) & 0x0F;
      break;

    case 4:
      hall_response = gpioExp1.readPort();
      button = (hall_response >> 12) & 0x0F;
      break;

    case 5:
      hall_response = gpioExp2.readPort();
      button = (hall_response) & 0x0F;
      invert_magnet = 1;
      break;

    case 6:
      hall_response = gpioExp2.readPort();
      button = (hall_response >> 4) & 0x0F;
      invert_magnet = 1;
      break;

    case 7:
      hall_response = gpioExp2.readPort();
      button = (hall_response >> 8) & 0x0F;
      invert_magnet = 1;
      break;

    case 8:
      hall_response = gpioExp2.readPort();
      button = (hall_response >> 12) & 0x0F;
      invert_magnet = 1;
      twisted = 1;
      break;

    case 9:
      hall_response = gpioExp3.readPort();
      button = (hall_response) & 0x0F;
      break;

    case 10:
      hall_response = gpioExp3.readPort();
      button = (hall_response >> 4) & 0x0F;
      break;

    case 11:
      hall_response = gpioExp3.readPort();
      button = (hall_response >> 8) & 0x0F;
      break;

    case 12:
      hall_response = gpioExp3.readPort();
      button = (hall_response >> 12) & 0x0F;
      break;

    case 13:
      hall_response = gpioExp4.readPort();
      button = (hall_response) & 0x0F;
      break;

    case 14:
      hall_response = gpioExp4.readPort();
      button = (hall_response >> 4) & 0x0F;
      break;

    case 15:
      hall_response = gpioExp4.readPort();
      button = (hall_response >> 8) & 0x0F;
      break;

    case 16:
      hall_response = gpioExp4.readPort();
      button = (hall_response >> 12) & 0x0F;
      break;

    default:
      hall_response = 0;
      button = 0;
      break;
    }

    // only bottom 3 bits have hall data
    button = button & 0x07;

    // untwist bits
    if (twisted)
    {
      t_button = ((button & 0x01) + ((button & 0x02) << 1) + ((button & 0x04) >> 1));
      //debug_printf("button %X, t_button is %X\n\r", button, t_button);
      button = t_button; 
    }

    debugPrintf("read button %X, hall = %X, button is %X\n\r", button_to_check, hall_response, button);

    // convert to button codes
    switch (button)
    {
      case 2:
        ret_val = invert_magnet ? PRIMO_MAGNET_FUNCTION : PRIMO_MAGNET_FUNCTION;
        break;
      case 3:
        ret_val = invert_magnet ? PRIMO_MAGNET_RIGHT : PRIMO_MAGNET_FORWARD;
        break;
      case 4:
        ret_val = invert_magnet ? PRIMO_MAGNET_FUNCTION : PRIMO_MAGNET_NONE;
        break;
      case 5:
        ret_val = invert_magnet ? PRIMO_MAGNET_LEFT : PRIMO_MAGNET_LEFT;
        break;
      case 6:
        ret_val = invert_magnet ? PRIMO_MAGNET_FORWARD : PRIMO_MAGNET_RIGHT;
        break;
      case 7:
        ret_val = PRIMO_MAGNET_NONE;
        break;
      default:
        ret_val = PRIMO_MAGNET_NONE;
        break;
    }

  debugPrintf("found button %X\n\r", ret_val);

  return ret_val;
}

