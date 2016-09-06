void setup() {
	Serial.begin(57600);
}

void loop() {
	unsigned char A[] = { 0x00, 0x0A, 0x20, 0x01, 0x01, 0x10,
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char BCC = checksum(A, sizeof(A)/sizeof(A[0]));
	Serial.print("Checksum: ");
	Serial.println(BCC, HEX);
	delay(5000);
}

unsigned char checksum(unsigned char A[], int numElements) {
	int i = 0;
	unsigned char BCC = A[0];

	for(i = 1; i < numElements; i++) {
		BCC = BCC ^ A[i];
	}

	return BCC;
}