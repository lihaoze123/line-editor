#include "active_zone.h"
#include "error.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace line_editor {

ActiveZone::ActiveZone(int maxLines)
    : head_(nullptr), tail_(nullptr), startLineNo_(1), lineCount_(0), maxLines_(maxLines) {
}

ActiveZone::~ActiveZone() {
    clear();
}

Line* ActiveZone::getLine(int relativeIndex) {
    if (relativeIndex < 0 || relativeIndex >= lineCount_) {
        return nullptr;
    }

    Line* current = head_;
    for (int i = 0; i < relativeIndex && current; ++i) {
        current = current->next();
    }

    return current;
}

Line* ActiveZone::getLineByNumber(int lineNo) {
    return findLine(lineNo);
}

int ActiveZone::getRelativeIndex(int lineNo) const {
    return lineNo - startLineNo_;
}

void ActiveZone::insert(int afterLineNo, const char* text) {
    Line* newLine = new Line(text);

    if (afterLineNo < startLineNo_) {
        newLine->setNext(head_);
        if (head_) {
            head_->setPrev(newLine);
        }
        head_ = newLine;
        if (!tail_) {
            tail_ = newLine;
        }
        lineCount_++;
    } else {
        Line* afterLine = findLine(afterLineNo);
        if (afterLine) {
            insertAfter(afterLine, newLine);
        } else {
            if (tail_) {
                tail_->setNext(newLine);
                newLine->setPrev(tail_);
            }
            tail_ = newLine;
            if (!head_) {
                head_ = newLine;
            }
            lineCount_++;
        }
    }

    if (lineCount_ > maxLines_) {
        Line* toRemove = head_;
        head_ = head_->next();
        if (head_) {
            head_->setPrev(nullptr);
        } else {
            tail_ = nullptr;
        }
        toRemove->setPrev(nullptr);
        toRemove->setNext(nullptr);
        delete toRemove;
        lineCount_--;
        startLineNo_++;
    }
}

void ActiveZone::deleteLine(int lineNo) {
    deleteRange(lineNo, lineNo);
}

void ActiveZone::deleteRange(int startLineNo, int endLineNo) {
    if (startLineNo > endLineNo) {
        throw EditorException(ErrorCode::INVALID_RANGE,
            "起始行号不能大于结束行号");
    }

    std::vector<Line*> toDelete;
    for (int no = startLineNo; no <= endLineNo; ++no) {
        Line* line = findLine(no);
        if (line) {
            toDelete.push_back(line);
        }
    }

    for (Line* line : toDelete) {
        removeLine(line);
    }
}

bool ActiveZone::replaceInLine(int lineNo, const char* oldStr, const char* newStr) {
    Line* line = findLine(lineNo);
    if (!line) {
        return false;
    }
    return line->replace(oldStr, newStr);
}

std::vector<int> ActiveZone::findPattern(const char* pattern) const {
    std::vector<int> results;
    Line* current = head_;
    int currentNo = startLineNo_;

    while (current) {
        if (current->contains(pattern)) {
            results.push_back(currentNo);
        }
        current = current->next();
        currentNo++;
    }

    return results;
}

std::string ActiveZone::display(int page) const {
    std::ostringstream oss;

    int startIdx = page * PAGE_SIZE;
    int endIdx = std::min(startIdx + PAGE_SIZE, lineCount_);

    Line* current = head_;
    int currentIdx = 0;
    int currentNo = startLineNo_;

    while (current && currentIdx < startIdx) {
        current = current->next();
        currentIdx++;
        currentNo++;
    }

    while (current && currentIdx < endIdx) {
        oss << std::setw(4) << currentNo << " " << current->getText() << "\n";
        current = current->next();
        currentIdx++;
        currentNo++;
    }

    return oss.str();
}

int ActiveZone::totalPages() const {
    return (lineCount_ + PAGE_SIZE - 1) / PAGE_SIZE;
}

void ActiveZone::clear() {
    Line* current = head_;
    while (current) {
        Line* next = current->next();
        delete current;
        current = next;
    }
    head_ = nullptr;
    tail_ = nullptr;
    lineCount_ = 0;
}

void ActiveZone::appendLine(Line* line) {
    if (!line) return;

    if (!head_) {
        head_ = line;
    } else {
        tail_->setNext(line);
        line->setPrev(tail_);
    }
    tail_ = line;
    lineCount_++;
}

Line* ActiveZone::removeFirst() {
    if (!head_) {
        return nullptr;
    }

    Line* first = head_;
    head_ = head_->next();
    if (head_) {
        head_->setPrev(nullptr);
    } else {
        tail_ = nullptr;
    }
    first->setPrev(nullptr);
    first->setNext(nullptr);
    lineCount_--;
    startLineNo_++;

    return first;
}

Line* ActiveZone::removeLast() {
    if (!tail_) {
        return nullptr;
    }

    Line* last = tail_;
    tail_ = tail_->prev();
    if (tail_) {
        tail_->setNext(nullptr);
    } else {
        head_ = nullptr;
    }
    last->setPrev(nullptr);
    last->setNext(nullptr);
    lineCount_--;

    return last;
}

void ActiveZone::insertAfter(Line* position, Line* newLine) {
    if (!position || !newLine) {
        return;
    }

    Line* next = position->next();

    position->setNext(newLine);
    newLine->setPrev(position);

    if (next) {
        newLine->setNext(next);
        next->setPrev(newLine);
    } else {
        tail_ = newLine;
    }

    lineCount_++;
}

void ActiveZone::removeLine(Line* line) {
    if (!line) {
        return;
    }

    Line* prev = line->prev();
    Line* next = line->next();

    if (prev) {
        prev->setNext(next);
    } else {
        head_ = next;
    }

    if (next) {
        next->setPrev(prev);
    } else {
        tail_ = prev;
    }

    line->setPrev(nullptr);
    line->setNext(nullptr);
    delete line;
    lineCount_--;
}

Line* ActiveZone::findLine(int lineNo) const {
    if (lineNo < startLineNo_ || lineNo >= startLineNo_ + lineCount_) {
        return nullptr;
    }

    int relativeIdx = lineNo - startLineNo_;
    Line* current = head_;

    for (int i = 0; i < relativeIdx && current; ++i) {
        current = current->next();
    }

    return current;
}

} // namespace line_editor
