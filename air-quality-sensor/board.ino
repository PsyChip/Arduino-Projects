
double GetTemp(void) {
  unsigned int wADC;
  double t;
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  delay(10);
  ADCSRA |= _BV(ADEN);  // enable the ADC
  ADCSRA |= _BV(ADSC);  // Start the ADC
  while (bit_is_set(ADCSRA, ADSC));
  wADC = ADCW;
  t = (wADC - 324.31 ) / 1.22;
  return (t);
}

long readVcc() {
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result;
  return result;
}

int availableMemory() {
  int size = 2048; // Use 2048 with ATmega328
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);
  free(buf);
  return size;
}

