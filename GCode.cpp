#include "GCode.hpp"

// -----------------------------------------------

GCode::Parameters GCode::Parse(char* t_buffer)
{
#ifdef COMMENT_SAFE
  char* comment = strchr(t_buffer, ';');
  if (comment != NULL)
    comment[0] = '\00';
#endif

  Parameters params;

#ifdef DEBUG_GCODE
  params.buffer = t_buffer;
#endif

  int length = strlen(t_buffer);

#ifdef LOOKFOR_CHECKSUM

  // Check if Line Number is provided
  if (t_buffer[0] == 'N')
  {
    int calculated_checksum = 0;

    char* checksum_pos = strchr(t_buffer, '*');
    int checksum = atoi(checksum_pos + 1);
    if (checksum_pos != NULL)
    {
      int c = 0;
      while (t_buffer[c] != '*')
      {
        calculated_checksum ^= t_buffer[c];
        c++;
      }
    }

    params.lineNumber = atol(t_buffer + 1);
    t_buffer = strchr(t_buffer, ' ');
    t_buffer++;

    // Wrong Checksum .. Error in Transmission ? We should ask for a line resend ..
    if (checksum != calculated_checksum)
    {
#ifdef DEBUG_CHECKSUM
      Serial.print("//Error Wrong Checksum : Expected ["); Serial.print(checksum); Serial.print("] Calculated ["); Serial.print(calculated_checksum); Serial.println("]");
#endif
      // We dont need to continue parsing an invalid buffer ..
      params.checksum = false;
      return params;
    }
  }
#else

  // We are not looking for checksums, but line number may still be provided and it will break the parsing if we dont check it ..
  if (t_buffer[0] == 'N')
  {
    params.lineNumber = atol(t_buffer + 1);
    t_buffer = strchr(t_buffer, ' ');
    t_buffer++;
  }
  
#endif

  for (int i = 0; i < length; i++)
  {
    if (t_buffer[i] >= 'a' && t_buffer[i] <= 'z')
    {
      t_buffer[i] = t_buffer[i] - 'a' + 'A';
    }
  }

  // We do dont parse comment nor anything else aother than G or M commands
  if (t_buffer[0] != 'G' && t_buffer[0] != 'M')
  {
    params.isInvalidComment = true;
    return params;
  }

  params.letter = t_buffer[0];
  params.code = (unsigned char)atoi(t_buffer + 1);

  t_buffer = strchr(t_buffer, ' ');
  t_buffer++;

  unsigned int currentIndex = 0;
  int counter = 0;

  if (params.letter == 'G')
  {
    char* pos;

    switch (params.code)
    {
      case 92 :
      case 1 :
      case 0 :
        pos = strchr(t_buffer, 'X');
        if (pos != NULL)
          params.x = atof(pos + 1);

        pos = strchr(t_buffer, 'Y');
        if (pos != NULL)
          params.y = atof(pos + 1);

        pos = strchr(t_buffer, 'Z');
        if (pos != NULL)
          params.z = atof(pos + 1);

        pos = strchr(t_buffer, 'E');
        if (pos != NULL)
          params.e = atof(pos + 1);

        if (params.code == 1)
        {
          pos = strchr(t_buffer, 'F');
          if (pos != NULL)
            params.f = atof(pos + 1);
        }

        break;

      case 28 :
        pos = strchr(t_buffer, 'X');
        if (pos != NULL)
          params.x = 1;

        pos = strchr(t_buffer, 'Y');
        if (pos != NULL)
          params.y = params.y = 1;

        pos = strchr(t_buffer, 'Z');
        if (pos != NULL)
          params.z = params.z = 1;
        break;

      case 4 :
        pos = strchr(t_buffer, 'S');
        if (pos != NULL)
          params.s = atof(pos + 1);

        pos = strchr(t_buffer, 'P');
        if (pos != NULL)
          params.p = atof(pos + 1);
        break;
    }
  }
  else // M
  {
    // there is no need to switch .. as we are taking about the same parameters for all the M commands we support
    // lots of them dont even use param,eters at all .. but its okay we'll just look for them
    char* pos;

    pos = strchr(t_buffer, 'S');
    if (pos != NULL)
      params.s = atof(pos + 1);

    pos = strchr(t_buffer, 'T');
    if (pos != NULL)
      params.t = atof(pos + 1);
  }
  return params;
}

// -----------------------------------------------

void GCode::Dump(GCode::Parameters t_params)
{
  Serial.println("Dump : ");

  if (t_params.isInvalidComment)
  {
    Serial.println("Invalid Command or comment line");
    return;
  }

#ifdef DEBUG_GCODE
  Serial.print("Buffer : "); Serial.println(t_params.buffer);
#endif

  Serial.println(t_params.letter);
  Serial.println((int)t_params.code);

  Serial.print("Line Number : "); Serial.println(t_params.lineNumber);
  Serial.print("Checksum : "); Serial.println(t_params.checksum);

  Serial.print("X : "); Serial.println(t_params.x);
  Serial.print("Y : "); Serial.println(t_params.y);
  Serial.print("Z : "); Serial.println(t_params.z);
  Serial.print("E : "); Serial.println(t_params.e);
  Serial.print("F : "); Serial.println(t_params.f);
  Serial.print("S : "); Serial.println(t_params.s);
  Serial.print("P : "); Serial.println(t_params.p);
  Serial.print("T : "); Serial.println(t_params.t);
  Serial.println();
}

// -----------------------------------------------

