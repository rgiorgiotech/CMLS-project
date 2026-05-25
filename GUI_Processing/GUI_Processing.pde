import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress myRemoteLocation;

float currentScale = 1.0;
float targetScale = 1.0;

// Lista dinamica che conterrà tutte le scintille attive sullo schermo
ArrayList<Scintilla> listaScintille = new ArrayList<Scintilla>();

void setup() {
  size(640, 360); 
  rectMode(CENTER);
  frameRate(125);
  
  oscP5 = new OscP5(this, 12000);
  myRemoteLocation = new NetAddress("127.0.0.1", 57120);
}

void draw() {
  // Sfondo scuro per far risaltare la brillantezza delle particelle
  background(20);
  
  // 1. AGGIORNA E DISEGNA LE SCINTILLE
  for (int i = listaScintille.size() - 1; i >= 0; i--) {
    Scintilla s = listaScintille.get(i);
    s.update();
    s.display();
    
    // Se la scintilla si è spenta del tutto (alpha <= 0), eliminala per non sovraccaricare la CPU
    if (s.isDead()) {
      listaScintille.remove(i);
    }
  }
  
  // 2. DISEGNA I RETTANGOLI (Manteniamo il vecchio effetto combinato al mouse)
  fill(255, 40); // Più trasparenti per non coprire le scintille
  noStroke();
  currentScale = lerp(currentScale, targetScale, 0.1);
  
  float baseDim1 = mouseY/2 + 10;
  float finalDim1 = baseDim1 * currentScale;
  rect(mouseX, height/2, finalDim1, finalDim1);
  
  int inverseX = width - mouseX;
  int inverseY = height - mouseY;
  float baseDim2 = (inverseY/2) + 10;
  float finalDim2 = baseDim2 * currentScale;
  rect(inverseX, height/2, finalDim2, finalDim2);
  
  // Invia dati mouse a SuperCollider
  OscMessage myMessage = new OscMessage("/pos");
  myMessage.add(mouseX / (float)width);
  myMessage.add(mouseY / (float)height);
  oscP5.send(myMessage, myRemoteLocation); 
}

// ASCOLTO DEI MESSAGGI DA SUPERCOLLIDER
void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.checkAddrPattern("/vmpkNota") == true) {
    int statoNota = theOscMessage.get(0).intValue(); // 1 = premuto, 0 = rilasciato
    int numeroNota = theOscMessage.get(1).intValue(); // Il valore MIDI della nota (0-127)
    
    if (statoNota == 1) {
      targetScale = 2.5; 
      
      // STABILISCO IL COLORE IN BASE AL REGISTRO
      color coloreScintilla;
      if (numeroNota > 60) {
        // REGISTRO ALTO: Azzurro Neon Elettrico ultra-brillante
        coloreScintilla = color(0, 255, 230); 
      } else {
        // REGISTRO GRAVE: Rosso mattone/fango, cupo e annerito
        coloreScintilla = color(55, 20, 10); 
      }
      
      // Quando premo il tasto, genero un'esplosione di 40 scintille dal fondo della finestra
      for (int i = 0; i < 40; i++) {
        // Le faccio partire da posizioni casuali sul lato inferiore (y = height)
        listaScintille.add(new Scintilla(random(0, width), height, coloreScintilla));
      }
    } else {
      targetScale = 1.0; 
    }
  }
}

// STRUTTURA DELLA SINGOLA SCINTILLA (Classe Oggetto)
class Scintilla {
  float x, y;
  float vx, vy;
  float alpha;
  color c;
  float dimensione;

  Scintilla(float startX, float startY, color coloreAssegnato) {
    x = startX;
    y = startY;
    // Velocità orizzontale casuale (sinistra/destra)
    vx = random(-2.5, 2.5);
    // Velocità verticale negativa per farle schizzare verso l'alto
    vy = random(-9, -3); 
    c = coloreAssegnato;
    alpha = 255; // Massima luminosità iniziale (Attacco dell'inviluppo)
    dimensione = random(3, 7);
  }

  void update() {
    x += vx;
    y += vy;
    
    // Una leggera gravità fa rallentare la loro salita verso l'alto
    vy += 0.12; 
    
    // DECADIMENTO DELL'INVILUPPO: ogni frame perdono luminosità
    alpha -= 3.5; 
  }

  void display() {
    noStroke();
    // Applichiamo il valore di alpha per sfumare l'intensità del colore
    fill(red(c), green(c), blue(c), alpha);
    ellipse(x, y, dimensione, dimensione);
  }

  // Controlla se la particella è svanita del tutto
  boolean isDead() {
    return (alpha <= 0);
  }
}
