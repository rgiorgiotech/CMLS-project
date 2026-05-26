import oscP5.*;
import netP5.*;

OscP5 oscP5;

final int COLS = 80;
final int ROWS = 50;
final int CELL_SIZE = 12;

boolean[][] grid = new boolean[COLS][ROWS];
boolean[][] nextGrid = new boolean[COLS][ROWS];

color[][] gridColor = new color[COLS][ROWS];
color[][] nextColor = new color[COLS][ROWS];

color[] pitchColors;

float worldDelay = 0.5;
float worldFeedback = 0.0;
float worldDamping = 0.0;
float worldDrive = 0.0;

int generationInterval = 8;
int frameCounter = 0;

void setup() {
  size(960, 600);
  frameRate(60);
  noStroke();

  oscP5 = new OscP5(this, 12000);

  pitchColors = new color[] {
    color(255, 80, 80),
    color(255, 130, 60),
    color(255, 200, 70),
    color(200, 255, 80),
    color(120, 255, 120),
    color(80, 255, 180),
    color(80, 220, 255),
    color(80, 150, 255),
    color(130, 100, 255),
    color(200, 90, 255),
    color(255, 90, 200),
    color(255, 120, 150)
  };

  clearGrid();
}

void draw() {
  background(8);

  updateGenerationInterval();

  frameCounter++;

  if (frameCounter >= generationInterval) {
    computeNextGeneration();
    frameCounter = 0;
  }

  drawGrid();
}

void updateGenerationInterval() {
  worldDelay = constrain(worldDelay, 0, 1);
  generationInterval = int(map(worldDelay, 0, 1, 2, 24));
}

void oscEvent(OscMessage msg) {
  println("OSC received: " + msg.addrPattern());

  if (msg.checkAddrPattern("/visual/note/on")) {
    int note = msg.get(0).intValue();
    seedRandomPattern(note);
  }

  if (msg.checkAddrPattern("/visual/note")) {
    int note = msg.get(0).intValue();
    seedRandomPattern(note);
  }

  if (msg.checkAddrPattern("/cc")) {
    int cc = msg.get(0).intValue();
    float value = constrain(msg.get(1).floatValue(), 0, 1);

    if (cc == 4) worldFeedback = value;
    if (cc == 5) worldDelay = value;
    if (cc == 6) worldDamping = value;
    if (cc == 7) worldDrive = value;

    println("CC " + cc + " = " + value);
  }
}

void seedRandomPattern(int note) {
  int pitchClass = note % 12;
  color c = pitchColors[pitchClass];

  int x = int(random(2, COLS - 4));
  int y = int(random(2, ROWS - 4));

  seedGlider(x, y, c);
}

void seedGlider(int x, int y, color c) {
  int[][] cells = {
    {1, 0},
    {2, 1},
    {0, 2},
    {1, 2},
    {2, 2}
  };

  for (int i = 0; i < cells.length; i++) {
    int nx = x + cells[i][0];
    int ny = y + cells[i][1];

    if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
      grid[nx][ny] = true;
      gridColor[nx][ny] = c;
    }
  }
}

void computeNextGeneration() {
  for (int x = 0; x < COLS; x++) {
    for (int y = 0; y < ROWS; y++) {
      int neighbors = countNeighbors(x, y);
      boolean alive = grid[x][y];

      boolean nextAlive = false;

      if (alive) {
        if (neighbors == 3) {
          nextAlive = true;
        } else if (neighbors == 2) {
          float survivalChance = 1.0 - worldDamping*0.35;
          nextAlive = random(1) < survivalChance;
        } else {
          nextAlive = false;
        }
      } else {
        if (neighbors == 3) {
          nextAlive = true;
        } else if (neighbors == 2) {
          float birthChance = worldFeedback * 0.20;
          nextAlive = random(1) < birthChance;
        } else if (neighbors == 4) {
          float birthChance = worldFeedback * 0.08;
          nextAlive = random(1) < birthChance;
        }
      }

      // DRIVE = pressione evolutiva/distorsione del sistema.
      // Non crea vita dal nulla.
      // Agisce solo sulle celle già vive:
      // - celle dense con 4 vicini possono sopravvivere
      // - celle fragili con 2 vicini possono collassare
      if (alive) {
        if (neighbors == 4) {
          float overdriveSurvivalChance = worldDrive * 0.12;

          if (random(1) < overdriveSurvivalChance) {
            nextAlive = true;
          }
        }

        if (neighbors == 2) {
          float instabilityChance = worldDrive * 0.08;

          if (random(1) < instabilityChance) {
            nextAlive = false;
          }
        }
      }

      nextGrid[x][y] = nextAlive;

      if (nextAlive) {
        if (alive) {
          nextColor[x][y] = gridColor[x][y];
        } else {
          nextColor[x][y] = inheritedColorFromNeighbors(x, y);
        }
      }
    }
  }

  boolean[][] tempGrid = grid;
  grid = nextGrid;
  nextGrid = tempGrid;

  color[][] tempColor = gridColor;
  gridColor = nextColor;
  nextColor = tempColor;

  clearNextGrid();
}

int countNeighbors(int x, int y) {
  int count = 0;

  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;

      int nx = x + dx;
      int ny = y + dy;

      if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
        if (grid[nx][ny]) count++;
      }
    }
  }

  return count;
}

color inheritedColorFromNeighbors(int x, int y) {
  float r = 0;
  float g = 0;
  float b = 0;
  int count = 0;

  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;

      int nx = x + dx;
      int ny = y + dy;

      if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && grid[nx][ny]) {
        color c = gridColor[nx][ny];
        r += red(c);
        g += green(c);
        b += blue(c);
        count++;
      }
    }
  }

  if (count == 0) {
    return color(255);
  }

  return color(r / count, g / count, b / count);
}

void drawGrid() {
  for (int x = 0; x < COLS; x++) {
    for (int y = 0; y < ROWS; y++) {
      if (grid[x][y]) {
        color c = gridColor[x][y];
        fill(red(c), green(c), blue(c));
        rect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
      }
    }
  }
}

void clearGrid() {
  for (int x = 0; x < COLS; x++) {
    for (int y = 0; y < ROWS; y++) {
      grid[x][y] = false;
      nextGrid[x][y] = false;
      gridColor[x][y] = color(255);
      nextColor[x][y] = color(255);
    }
  }
}

void clearNextGrid() {
  for (int x = 0; x < COLS; x++) {
    for (int y = 0; y < ROWS; y++) {
      nextGrid[x][y] = false;
      nextColor[x][y] = color(255);
    }
  }
}
