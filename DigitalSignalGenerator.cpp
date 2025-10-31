#include <iostream>
#include <cmath>
#include <cstring>
#include <GLUT/glut.h> //this library is of macos hence will be worked on macos only
using namespace std;

int* bufferPtr = nullptr;
int bufferSize = 0;
char headerLabel[120] = "";
bool halfCycle = false;

void codeNRZL(char* inputBits, int* encodedBits, int len) {
    for (int i = 0; i < len; i++) encodedBits[i] = (inputBits[i] == '1') ? 1 : -1;
}

void codeNRZI(char* inputBits, int* encodedBits, int len) {
    int voltage = -1;
    for (int i = 0; i < len; i++) {
        if (inputBits[i] == '1') voltage = -voltage;
        encodedBits[i] = voltage;
    }
}

void codeManchester(char* inputBits, int* encodedBits, int len) {
    for (int i = 0; i < len; i++) {
        if (inputBits[i] == '0') {
            encodedBits[2*i] = 1;
            encodedBits[2*i + 1] = -1;
        } else {
            encodedBits[2*i] = -1;
            encodedBits[2*i + 1] = 1;
        }
    }
}

void codeDiffManchester(char* inputBits, int* encodedBits, int len) {
    int level = -1;
    for (int i = 0; i < len; i++) {
        if (inputBits[i] == '0') {
            level = -level;
            encodedBits[2*i] = level;
            encodedBits[2*i + 1] = -level;
        } else {
            encodedBits[2*i] = level;
            encodedBits[2*i + 1] = -level;
        }
        level = -level;
    }
}

void codeAMI(char* inputBits, int* encodedBits, int len) {
    int polarity = 1;
    for (int i = 0; i < len; i++) {
        if (inputBits[i] == '0') encodedBits[i] = 0;
        else {
            encodedBits[i] = polarity;
            polarity = -polarity;
        }
    }
}

void scrambleB8ZS(int* arr, int n) {
    int polarity = 1;
    for (int i = 0; i <= n - 8; i++) {
        if (arr[i] != 0) polarity = arr[i];
        bool zeroSeq = true;
        for (int j = i; j < i + 8; j++) if (arr[j] != 0) { zeroSeq = false; break; }
        if (zeroSeq) {
            arr[i+3] = polarity;
            arr[i+4] = -polarity;
            arr[i+6] = polarity;
            arr[i+7] = -polarity;
            polarity = -polarity;
            i += 7;
        }
    }
}

void scrambleHDB3(int* arr, int n) {
    int ones = 0, polarity = 1;
    for (int i = 0; i <= n - 4; i++) {
        if (arr[i] != 0) { polarity = arr[i]; ones++; }
        bool zeros = true;
        for (int j = i; j < i + 4; j++) if (arr[j] != 0) { zeros = false; break; }
        if (zeros) {
            if (ones % 2 == 0) { arr[i] = -polarity; arr[i+3] = polarity; }
            else arr[i+3] = polarity;
            polarity = -polarity;
            ones = 1;
            i += 3;
        }
    }
}

int encodePCM(double* analog, int samples, char* output, int bits) {
    double maxV = analog[0], minV = analog[0];
    for (int i = 1; i < samples; i++) {
        if (analog[i] > maxV) maxV = analog[i];
        if (analog[i] < minV) minV = analog[i];
    }
    int levels = pow(2, bits);
    double gap = (maxV - minV) / levels;
    int pos = 0;
    for (int i = 0; i < samples; i++) {
        int quant = (int)((analog[i] - minV) / gap);
        if (quant >= levels) quant = levels - 1;
        for (int j = bits - 1; j >= 0; j--) output[pos++] = ((quant >> j) & 1) ? '1' : '0';
    }
    output[pos] = '\0';
    return pos;
}

int encodeDelta(double* analog, int samples, char* output) {
    double pred = 0, step = 0.5;
    for (int i = 0; i < samples; i++) {
        if (analog[i] > pred) { output[i] = '1'; pred += step; }
        else { output[i] = '0'; pred -= step; }
    }
    output[samples] = '\0';
    return samples;
}

void detectPalindrome(char* seq, int n) {
    int maxLen = 1, start = 0;
    char* tmp = new char[2*n + 3];
    int k = 0; tmp[k++] = '^';
    for (int i = 0; i < n; i++) { tmp[k++] = '|'; tmp[k++] = seq[i]; }
    tmp[k++] = '|'; tmp[k++] = '$';
    int* pArr = new int[k]; memset(pArr, 0, sizeof(int)*k);
    int c = 0, r = 0;
    for (int i = 1; i < k - 1; i++) {
        int mirr = 2*c - i;
        if (i < r) pArr[i] = min(r - i, pArr[mirr]);
        while (tmp[i + pArr[i] + 1] == tmp[i - pArr[i] - 1]) pArr[i]++;
        if (i + pArr[i] > r) { c = i; r = i + pArr[i]; }
        if (pArr[i] > maxLen) { maxLen = pArr[i]; start = (i - pArr[i]) / 2; }
    }
    cout << "\nLongest Palindrome Segment: ";
    for (int i = start; i < start + maxLen; i++) cout << seq[i];
    cout << " (Size: " << maxLen << ")\n";
    delete[] tmp; delete[] pArr;
}

void printZeros(int* arr, int n) {
    int maxZero = 0, count = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] == 0) count++;
        else { maxZero = max(maxZero, count); count = 0; }
    }
    maxZero = max(maxZero, count);
    if (maxZero > 0) cout << "Zero sequence length: " << maxZero << endl;
}

void textDraw(float x, float y, const char* txt, void* font) {
    glRasterPos2f(x, y);
    for (int i = 0; txt[i] != '\0'; i++) glutBitmapCharacter(font, txt[i]);
}

void graphRender() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (!bufferPtr || bufferSize == 0) { glFlush(); return; }
    glColor3f(0, 0, 1);
    glBegin(GL_LINE_STRIP);
    float step = 1.8 / bufferSize;
    float scaleY = 0.35;
    for (int i = 0; i < bufferSize; i++) {
        float x1 = -0.9 + i * step;
        float x2 = -0.9 + (i + 1) * step;
        float y = bufferPtr[i] * scaleY;
        glVertex2f(x1, y);
        glVertex2f(x2, y);
        if (i < bufferSize - 1) {
            float y2 = bufferPtr[i+1] * scaleY;
            glVertex2f(x2, y);
            glVertex2f(x2, y2);
        }
    }
    glEnd();
    glColor3f(0, 0, 0);
    textDraw(-0.8, 0.85, headerLabel, GLUT_BITMAP_HELVETICA_18);
    glFlush();
}

void setupScene() {
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
}

void launchSignal(int* arr, int len, const char* lbl, bool man) {
    bufferPtr = arr;
    bufferSize = len;
    strcpy(headerLabel, lbl);
    halfCycle = man;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    cout << "Signal Encoder for macOS\n";
    cout << "Select mode: 1) Digital  2) Analog\n> ";
    int mode; cin >> mode;

    char bitData[1000];
    int bitLen = 0;

    if (mode == 2) {
        cout << "1) PCM  2) Delta Modulation\n> ";
        int opt; cin >> opt;
        cout << "Enter sample count: ";
        int s; cin >> s;
        double* vals = new double[s];
        cout << "Input values: ";
        for (int i = 0; i < s; i++) cin >> vals[i];
        if (opt == 1) {
            cout << "Bits per sample: ";
            int b; cin >> b;
            bitLen = encodePCM(vals, s, bitData, b);
        } else bitLen = encodeDelta(vals, s, bitData);
        delete[] vals;
    } else {
        cout << "Binary stream: ";
        cin >> bitData;
        bitLen = strlen(bitData);
    }

    detectPalindrome(bitData, bitLen);

    cout << "\nChoose scheme:\n1.NRZ-L\n2.NRZ-I\n3.Manchester\n4.Diff Manchester\n5.AMI\n> ";
    int enc; cin >> enc;

    int* wave = nullptr;
    int lenOut = 0;
    char label[100];
    bool mFlag = false;

    switch (enc) {
        case 1: lenOut = bitLen; wave = new int[lenOut]; codeNRZL(bitData, wave, bitLen); strcpy(label, "NRZ-L"); break;
        case 2: lenOut = bitLen; wave = new int[lenOut]; codeNRZI(bitData, wave, bitLen); strcpy(label, "NRZ-I"); break;
        case 3: lenOut = bitLen * 2; wave = new int[lenOut]; codeManchester(bitData, wave, bitLen); strcpy(label, "Manchester"); mFlag = true; break;
        case 4: lenOut = bitLen * 2; wave = new int[lenOut]; codeDiffManchester(bitData, wave, bitLen); strcpy(label, "Diff Manchester"); mFlag = true; break;
        case 5:
            lenOut = bitLen; wave = new int[lenOut]; codeAMI(bitData, wave, bitLen); strcpy(label, "AMI");
            cout << "Add scrambling? (1=Yes,0=No): "; int scr; cin >> scr;
            if (scr == 1) {
                cout << "1) B8ZS  2) HDB3\n> "; int type; cin >> type;
                if (type == 1) { scrambleB8ZS(wave, lenOut); strcpy(label, "AMI+B8ZS"); }
                else { scrambleHDB3(wave, lenOut); strcpy(label, "AMI+HDB3"); }
                printZeros(wave, lenOut);
            }
            break;
    }

    cout << "\nEncoded sequence:\n";
    for (int i = 0; i < lenOut; i++) cout << wave[i] << " ";
    cout << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1000, 600);
    glutCreateWindow("macOS Signal Display");
    setupScene();
    launchSignal(wave, lenOut, label, mFlag);
    glutDisplayFunc(graphRender);
    cout << "Drawing waveform...\n";
    glutMainLoop();
    delete[] wave;
    return 0;
}

