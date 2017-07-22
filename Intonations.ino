void basicTone() {
  digitalWriteFast(virtbtn0, LOW);
  delay(130);
  digitalWriteFast(virtbtn0, HIGH);
}

void errorTone() {
  digitalWriteFast(virtbtn0, LOW);
  delay(130);
  digitalWriteFast(virtbtn0, HIGH);
}


void victoryTone () {
  digitalWriteFast(virtbtn1, LOW);
  delay(130);
  digitalWriteFast(virtbtn1, HIGH);
}

void segStartTone () {
  basicTone();
}

void segEndTone () {
  basicTone();
}




