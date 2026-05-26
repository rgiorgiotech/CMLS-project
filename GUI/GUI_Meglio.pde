import oscP5.*;
import netP5.*;

OscP5 oscP5;

color coloreSfondoCorrente;
color coloreSfondoBase = color(10); 

int[] ultimiValoriCC = new int[128]; 
ArrayList<Scintilla> listaScintille = new ArrayList<Scintilla>();

boolean[] layersAttivi = new boolean[8];
color[] coloriLayer = new color[8];

boolean setupCompletato = false;

void setup() {
  size(960, 540); 
  
  frameRate(125);
  oscP5 = new OscP5(this, 12000);
  coloreSfondoCorrente = coloreSfondoBase;
  
  for (int i = 0; i < 128; i++) {
    ultimiValoriCC[i] = 64;
  }
  
  // Palette colori ufficiale dei Layer
  coloriLayer[0] = color(0, 191, 255);   // Layer 1: Sine (Cyan)
  coloriLayer[1] = color(255, 69, 0);    // Layer 2: Saw (Arancione)
  coloriLayer[2] = color(255, 0, 128);   // Layer 3: Pulse (Magenta)
  coloriLayer[3] = color(255, 215, 0);   // Layer 4: FM Bell (Oro)
  coloriLayer[4] = color(240, 240, 240); // Layer 5: Noise (Argento)
  coloriLayer[5] = color(138, 43, 226);  // Layer 6: Sub (Viola)
  coloriLayer[6] = color(50, 205, 50);   // Layer 7: Glitch (Verde Neon)
  coloriLayer[7] = color(0, 128, 128);   // Layer 8: Pad/Drone (Ottanio)
  
  setupCompletato = true;
}

void draw() {
  if (!setupCompletato) return; 
  
  // Calcola il mix dei colori in base a quanti e quali layer sono attivi
  coloreSfondoBase = calcolaColoreBaseMisto();
  
  // Sfumatura fluida verso il colore target (0.03 rende il ritorno più morbido ed elegante)
  coloreSfondoCorrente = lerpColor(coloreSfondoCorrente, coloreSfondoBase, 0.03);
  noStroke();
  
  fill(red(coloreSfondoCorrente), green(coloreSfondoCorrente), blue(coloreSfondoCorrente), 60); 
  rect(0, 0, width, height);
  
  for (int i = listaScintille.size() - 1; i >= 0; i--) {
    Scintilla s = listaScintille.get(i);
    s.update();
    s.display();
    
    if (s.isDead()) {
      listaScintille.remove(i);
    }
  }
}

void oscEvent(OscMessage theOscMessage) {
  if (!setupCompletato) return; 
  
  // RICEZIONE PAD (Cambio stato dei layer)
  if (theOscMessage.checkAddrPattern("/visual/state") == true) {
    String layerName = theOscMessage.get(0).stringValue(); 
    int statoLayer = theOscMessage.get(1).intValue();       
    
    int layerIndex = Integer.parseInt(layerName.split("_")[1]) - 1;
    
    if (layerIndex >= 0 && layerIndex < 8) {
      layersAttivi[layerIndex] = (statoLayer == 1);
      
      // Quando attivi lo strumento, pioggia SOLO dall'alto
      if (statoLayer == 1) {
        color colorePad = coloriLayer[layerIndex];
        float xSorgente = map(layerIndex, 0, 7, width * 0.1, width * 0.9);
        
        int numeroScintillePad = (int)random(15, 30);
        for (int i = 0; i < numeroScintillePad; i++) {
          listaScintille.add(new Scintilla(xSorgente + random(-30, 30), 0, random(-2.0, 2.0), random(1.5, 4.5), colorePad));
        }
      }
    }
  }
  
  // GESTIONE NOTE
  if (theOscMessage.checkAddrPattern("/vmpkNota") == true) {
    int statoNota = theOscMessage.get(0).intValue();  
    int numeroNota = theOscMessage.get(1).intValue(); 
    int velocity = (theOscMessage.typetag().length() >= 3) ? theOscMessage.get(2).intValue() : 100;
    
    if (statoNota == 1) {
      float fattoreSfumatura = map(numeroNota, 36, 96, 0.0, 1.0);
      fattoreSfumatura = constrain(fattoreSfumatura, 0.0, 1.0); 
      color coloreNota = lerpColor(color(0, 150, 255), color(255, 50, 50), fattoreSfumatura);
      
      int numeroScintille = (int)map(velocity, 0, 127, 10, 35);
      for (int i = 0; i < numeroScintille; i++) {
        listaScintille.add(new Scintilla(random(0, width), height, random(-2.5, 2.5), random(-5.0, -1.5) * map(velocity,0,127,0.8,2.5), coloreNota));
      }
    }
  }
  
  // GESTIONE CC (Manopole - Ora pulito e privo di Pad!)
  if (theOscMessage.checkAddrPattern("/vmpkCC") == true) {
    int ccNumber = theOscMessage.get(0).intValue();
    int value = theOscMessage.get(1).intValue(); 
    
    if (ccNumber >= 0 && ccNumber < 128) {
      if (value > ultimiValoriCC[ccNumber]) {
        color coloreVerde = color(50, 255, 100);
        int scintilleCC = (int)random(2, 6);
        for (int i = 0; i < scintilleCC; i++) {
          listaScintille.add(new Scintilla(width, random(0, height), random(-7, -3), random(-2, 2), coloreVerde));
        }
      } 
      else if (value < ultimiValoriCC[ccNumber]) {
        color coloreGiallo = color(255, 220, 50);
        int scintilleCC = (int)random(2, 6);
        for (int i = 0; i < scintilleCC; i++) {
          listaScintille.add(new Scintilla(0, random(0, height), random(3, 7), random(-2, 2), coloreGiallo));
        }
      }
      ultimiValoriCC[ccNumber] = value;
    }
  }
}

// FUNZIONE PER IL MIX DEI COLORI DEI LAYER ATTIVI
color calcolaColoreBaseMisto() {
  int contatoreLayerAttivi = 0;
  float sommaR = 0, sommaG = 0, sommaB = 0;
  
  for (int i = 0; i < 8; i++) {
    if (layersAttivi[i]) {
      sommaR += red(coloriLayer[i]);
      sommaG += green(coloriLayer[i]);
      sommaB += blue(coloriLayer[i]);
      contatoreLayerAttivi++;
    }
  }
  
  // Se nessun strumento è attivo, lo sfondo base torna nero (valore 10)
  if (contatoreLayerAttivi == 0) {
    return color(10); 
  } else {
    // Fa la media dei canali RGB dei soli layer attualmente accesi
    return color(sommaR / contatoreLayerAttivi, sommaG / contatoreLayerAttivi, sommaB / contatoreLayerAttivi);
  }
}

class Scintilla {
  float x, y, vx, vy, alpha, dimensioneIniziale, dimensioneCorrente;
  color c;
  float noiseOffsetX, noiseOffsetY;
  boolean vieneDallAlto;

  Scintilla(float startX, float startY, float velX, float velY, color coloreAssegnato) {
    x = startX;
    y = startY;
    vx = velX; 
    vy = velY; 
    c = coloreAssegnato;
    alpha = 255;
    dimensioneIniziale = random(3, 8);
    dimensioneCorrente = dimensioneIniziale;
    noiseOffsetX = random(1000);
    noiseOffsetY = random(1000);
    vieneDallAlto = (startY == 0); 
  }

  void update() {
    float ventoX = map(noise(noiseOffsetX), 0, 1, -0.4, 0.4);
    float ventoY = map(noise(noiseOffsetY), 0, 1, -0.4, 0.4);
    vx += ventoX;
    vy += ventoY;
    
    x += vx;
    y += vy;
    
    if (vieneDallAlto) {
      vy += 0.08; 
    } else {
      vy += 0.15; 
    }
    
    alpha -= 3.5; 
    noiseOffsetX += 0.05;
    noiseOffsetY += 0.05;
    dimensioneCorrente = map(alpha, 0, 255, 0, dimensioneIniziale);
  }

  void display() {
    noStroke();
    fill(red(c), green(c), blue(c), alpha * 0.25);
    ellipse(x, y, dimensioneCorrente * 3.5, dimensioneCorrente * 3.5);
    fill(min(red(c) + 50, 255), min(green(c) + 50, 255), min(blue(c) + 50, 255), alpha);
    ellipse(x, y, dimensioneCorrente, dimensioneCorrente);
  }

  boolean isDead() {
    return (alpha <= 0 || x < -50 || x > width + 50 || y > height + 50);
  }
}
