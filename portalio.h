#pragma once

unsigned char *ReadSkylanderFromPortal(void);
bool WriteSkylanderToPortal(unsigned char *encrypted_new_data, unsigned char *encrypted_old_data);