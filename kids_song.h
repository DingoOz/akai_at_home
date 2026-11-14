// Lessons for Piano Pals: from "what is a key?" to the whole Jingle Bells chorus, one small step at a time.
#pragma once
#include <QColor>
#include <QString>
#include <vector>

// Note colours (like coloured bell sets), names, and which finger plays them with the thumb on C.
inline const QColor kColour[12] = {"#e53935", "#888", "#fb8c00", "#888", "#fdd835", "#43a047", "#888",
                                   "#00acc1", "#888", "#5e35b1", "#888", "#d81b60"};  // C red, D orange, E yellow...
inline const char *kName[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
inline const char *kColourName[12] = {"RED", "", "ORANGE", "", "YELLOW", "GREEN", "", "BLUE", "", "PURPLE", "", "PINK"};
inline const int kWhite[8] = {0, 2, 4, 5, 7, 9, 11, 12};  // on-screen keys: C D E F G A B C (12 = top C)
inline const int kBlack[5] = {1, 3, 6, 8, 10};
inline const int kBlackAfter[5] = {0, 1, 3, 4, 5};  // black key sits between white key i and i+1
inline int finger(int pc) { return pc == 0 ? 1 : pc == 2 ? 2 : pc == 4 ? 3 : pc == 5 ? 4 : pc == 7 ? 5 : 0; }

struct Note { int pc; double beats; const char *word; };  // pc: 0 = C, 2 = D, 4 = E ... ; -1 = any key
struct Lesson {
    QString title, say;
    std::vector<Note> notes;
    bool showC = false;  // point out the two black keys next to C
};

inline std::vector<Note> operator+(std::vector<Note> a, const std::vector<Note> &b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

inline std::vector<Lesson> lessons() {
    const std::vector<Note> bells = {{4, 1, "Jin-"}, {4, 1, "gle"}, {4, 2, "bells,"},
                                     {4, 1, "jin-"}, {4, 1, "gle"}, {4, 2, "bells,"}};
    const std::vector<Note> way = {{4, 1, "jin-"}, {7, 1, "gle"}, {0, 1.5, "all"}, {2, .5, "the"}, {4, 4, "way!"}};
    const std::vector<Note> fun = {{5, 1, "Oh"}, {5, 1, "what"}, {5, 1.5, "fun"}, {5, .5, "it"}, {5, 1, "is"},
                                   {4, 1, "to"}, {4, 1, "ride"}, {4, .5, "in"}, {4, .5, "a"}};
    const std::vector<Note> hey = {{4, 1, "one"}, {2, 1, "horse"}, {2, 1, "o-"}, {4, 1, "pen"},
                                   {2, 2, "sleigh,"}, {7, 2, "hey!"}};
    const std::vector<Note> end = {{7, 1, "one"}, {7, 1, "horse"}, {5, 1, "o-"}, {2, 1, "pen"}, {0, 4, "sleigh!"}};
    return {
        {"Hello, piano!", "Press any key you like. Listen to the sound it makes!",
         {{-1, 1, "any"}, {-1, 1, "any"}, {-1, 1, "any"}, {-1, 1, "any"}}},
        {"Find the red key: C",
         "Find two black keys sitting together. The white key just to their left is C. Press C!",
         {{0, 1, "C"}, {0, 1, "C"}, {0, 1, "C"}}, true},
        {"Meet your fingers",
         "Put your thumb on C. Each finger gets the next key. Play the numbers up 1 2 3 4 5 and back down!",
         {{0, 1, "1"}, {2, 1, "2"}, {4, 1, "3"}, {5, 1, "4"}, {7, 1, "5"},
          {5, 1, "4"}, {4, 1, "3"}, {2, 1, "2"}, {0, 2, "1"}}},
        {"Jingle bells!", "Play the yellow key with finger 3.", bells},
        {"...jingle all the way!", "Your fingers jump about a bit here. Follow the arrow!", way},
        {"Oh what fun...", "Lots of green F, then yellow E. Sing along if you like!", fun + hey},
        {"...one horse open sleigh!", "Nearly the same as before, but this time it walks down to C at the end.",
         fun + end},
        {"The whole song!", "You know every part. Now play it all the way through!",
         bells + way + fun + hey + bells + way + fun + end},
    };
}
