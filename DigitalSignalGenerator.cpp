#include <iostream>
#include <cmath>
#include <cstring>
#include <GLUT/glut.h>
using namespace std;

int* waveBuffer = nullptr;
int waveSize = 0;
char waveLabel[120] = "";
bool isManchester = false;

// ---------------------- Encoding Functions ----------------------
void NRZLencode(char* bits, int* signal, int n) {
    for (int i = 0; i < n; i++) signal[i] = (bits[i] == '1') ? 1 : -1;
}

void NRZIencode(char* bits, int* signal, int n) {
    int current = -1;
    for (int i = 0; i < n; i++) {
        if (bits[i] == '1') current = -current;
        signal[i] = current;
    }
}

void ManchesterEncode(char* bits, int* signal, int n) {
    for (int i = 0; i < n; i++) {
        if (bits[i] == '0') { signal[2*i] = 1; signal[2*i+1] = -1; }
        else { signal[2*i] = -1; signal[2*i+1] = 1; }
    }
}

void DiffManchesterEncode(char* bits, int* signal, int n) {
    int level = -1;
    for (int i = 0; i < n; i++) {
        if (bits[i] == '0') level = -level;
        signal[2*i] = level;
        signal[2*i+1] = -level;
        level = -level;
    }
}

void AMIencode(char* bits, int* signal, int n) {
    int polarity = 1;
    for (int i = 0; i < n; i++) {
        if (bits[i] == '0') signal[i] = 0;
        else { signal[i] = polarity; polarity = -polarity; }
    }
}

// ---------------------- Scrambling Functions ----------------------
void B8ZSscramble(int* signal, int len) {
    int lastPolarity = 1;
    for (int i = 0; i <= len - 8; ++i) {
        if (signal[i] != 0) lastPolarity = signal[i];

        bool allZeros = true;
        for (int j = 0; j < 8; ++j) {
            if (signal[i + j] != 0) { allZeros = false; break; }
        }
        if (!allZeros) continue;

        int V = lastPolarity;    // violation = same polarity
        int B = -V;              // opposite polarity

        // B8ZS pattern
        signal[i + 3] = V;
        signal[i + 4] = B;
        signal[i + 5] = 0;
        signal[i + 6] = B;
        signal[i + 7] = V;

        lastPolarity = signal[i + 7];
        i += 7;
    }
}

// ✅ Your version of HDB3 integrated
void scrambleHDB3(char* bits, int* encoded, int n) {
    int zeroCount = 0;
    bool flag = true;      // toggles between B00V / 000V
    bool prev = false;     // tracks AMI polarity (false=-1, true=+1)

    for (int i = 0; i < n; i++) {
        if (bits[i] == '1') {
            encoded[i] = prev ? -1 : 1;
            zeroCount = 0;
            flag = !flag;
            prev = !prev;
        } else {
            encoded[i] = 0;
            zeroCount++;
        }

        if (zeroCount == 4) {
            if (flag) {
                // B00V pattern
                encoded[i-3] = prev ? -1 : 1;
                encoded[i] = prev ? -1 : 1;
            } else {
                // 000V pattern
                encoded[i] = prev ? 1 : -1;
            }
            zeroCount = 0;
            flag = true;
            prev = (encoded[i] > 0);
        }
    }
}

// ---------------------- Analog Encoding ----------------------
int PCMencode(double* analog, int samples, char* output, int bits) {
    double maxV = analog[0], minV = analog[0];
    for (int i = 1; i < samples; i++) {
        if (analog[i] > maxV) maxV = analog[i];
        if (analog[i] < minV) minV = analog[i];
    }
    int levels = pow(2, bits);
    double step = (maxV - minV) / levels;
    int pos = 0;
    for (int i = 0; i < samples; i++) {
        int quant = (int)((analog[i] - minV) / step);
        if (quant >= levels) quant = levels - 1;
        for (int j = bits - 1; j >= 0; j--) output[pos++] = ((quant >> j) & 1) ? '1' : '0';
    }
    output[pos] = '\0';
    return pos;
}

int DeltaModulate(double* analog, int samples, char* output) {
    double pred = 0, delta = 0.5;
    for (int i = 0; i < samples; i++) {
        if (analog[i] > pred) { output[i] = '1'; pred += delta; }
        else { output[i] = '0'; pred -= delta; }
    }
    output[samples] = '\0';
    return samples;
}

// ---------------------- Helpers ----------------------
void longestPalindrome(char* seq, int n) {
    int maxLen = 1, start = 0;
    char* mod = new char[2*n + 3];
    int k = 0; mod[k++] = '^';
    for (int i = 0; i < n; i++) { mod[k++] = '|'; mod[k++] = seq[i]; }
    mod[k++] = '|'; mod[k++] = '$';
    int* P = new int[k]; memset(P, 0, sizeof(int)*k);
    int center = 0, right = 0;
    for (int i = 1; i < k - 1; i++) {
        int mirror = 2*center - i;
        if (i < right) P[i] = min(right - i, P[mirror]);
        while (mod[i + P[i] + 1] == mod[i - P[i] - 1]) P[i]++;
        if (i + P[i] > right) { center = i; right = i + P[i]; }
        if (P[i] > maxLen) { maxLen = P[i]; start = (i - P[i]) / 2; }
    }
    cout << "\nLongest Palindrome in data: ";
    for (int i = start; i < start + maxLen; i++) cout << seq[i];
    cout << " (Length: " << maxLen << ")\n";
    delete[] mod; delete[] P;
}

void printLongestZeros(int* arr, int n) {
    int maxZ = 0, cur = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] == 0) cur++;
        else { maxZ = max(maxZ, cur); cur = 0; }
    }
    maxZ = max(maxZ, cur);
    if (maxZ > 0) cout << "Longest zero sequence: " << maxZ << "\n";
}

// ---------------------- Visualization ----------------------
void drawText(float x, float y, const char* text, void* font) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) glutBitmapCharacter(font, text[i]);
}

void renderWave() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (!waveBuffer || waveSize == 0) { glFlush(); return; }
    glColor3f(0, 0, 1);
    glBegin(GL_LINE_STRIP);
    float step = 1.8 / waveSize;
    float scale = 0.35;
    for (int i = 0; i < waveSize; i++) {
        float x1 = -0.9 + i * step;
        float x2 = -0.9 + (i + 1) * step;
        float y = waveBuffer[i] * scale;
        glVertex2f(x1, y);
        glVertex2f(x2, y);
        if (i < waveSize - 1) {
            float y2 = waveBuffer[i+1] * scale;
            glVertex2f(x2, y);
            glVertex2f(x2, y2);
        }
    }
    glEnd();
    glColor3f(0, 0, 0);
    drawText(-0.8, 0.85, waveLabel, GLUT_BITMAP_HELVETICA_18);
    glFlush();
}

void setupGL() {
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
}

void visualize(int* sig, int len, const char* lbl, bool man) {
    waveBuffer = sig; waveSize = len;
    strcpy(waveLabel, lbl);
    isManchester = man;
    glutPostRedisplay();
}

// ---------------------- Main ----------------------
int main(int argc, char** argv) {
    cout << "Digital Signal Generator (macOS)\n";
    cout << "Input Type: 1) Digital  2) Analog\n> ";
    int mode; cin >> mode;

    char bits[1000];
    int len = 0;

    if (mode == 2) {
        cout << "1) PCM  2) Delta Modulation\n> ";
        int opt; cin >> opt;
        cout << "Number of samples: ";
        int s; cin >> s;
        double* analog = new double[s];
        cout << "Enter sample values: ";
        for (int i = 0; i < s; i++) cin >> analog[i];
        if (opt == 1) {
            cout << "Bits per sample: ";
            int b; cin >> b;
            len = PCMencode(analog, s, bits, b);
        } else len = DeltaModulate(analog, s, bits);
        delete[] analog;
    } else {
        cout << "Binary stream: ";
        cin >> bits;
        len = strlen(bits);
    }

    longestPalindrome(bits, len);

    cout << "\nSelect Encoding Scheme:\n1.NRZ-L\n2.NRZ-I\n3.Manchester\n4.Diff Manchester\n5.AMI\n> ";
    int choice; cin >> choice;

    int* sig = nullptr;
    int sigLen = 0;
    char lbl[100];
    bool man = false;

    switch (choice) {
        case 1:
            sigLen = len; sig = new int[sigLen];
            NRZLencode(bits, sig, len);
            strcpy(lbl, "NRZ-L");
            break;

        case 2:
            sigLen = len; sig = new int[sigLen];
            NRZIencode(bits, sig, len);
            strcpy(lbl, "NRZ-I");
            break;

        case 3:
            sigLen = len * 2; sig = new int[sigLen];
            ManchesterEncode(bits, sig, len);
            strcpy(lbl, "Manchester");
            man = true;
            break;

        case 4:
            sigLen = len * 2; sig = new int[sigLen];
            DiffManchesterEncode(bits, sig, len);
            strcpy(lbl, "Diff Manchester");
            man = true;
            break;

        case 5:
            sigLen = len; sig = new int[sigLen];
            cout << "Apply scrambling? (1=Yes,0=No): ";
            int scr; cin >> scr;
            if (scr == 1) {
                cout << "1) B8ZS  2) HDB3\n> ";
                int type; cin >> type;
                if (type == 1) {
                    AMIencode(bits, sig, len);
                    B8ZSscramble(sig, sigLen);
                    strcpy(lbl, "AMI + B8ZS");
                } else {
                    scrambleHDB3(bits, sig, len);
                    strcpy(lbl, "AMI + HDB3");
                }
                printLongestZeros(sig, sigLen);
            } else {
                AMIencode(bits, sig, len);
                strcpy(lbl, "AMI");
            }
            break;
    }

    cout << "\nEncoded signal:\n";
    for (int i = 0; i < sigLen; i++) cout << sig[i] << " ";
    cout << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1000, 600);
    glutCreateWindow("Digital Signal Visualization (macOS)");
    setupGL();
    visualize(sig, sigLen, lbl, man);
    glutDisplayFunc(renderWave);
    cout << "Rendering waveform...\n";
    glutMainLoop();

    delete[] sig;
    return 0;
}
