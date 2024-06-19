# lousy port of https://github.com/Chlorophytus/chlorobot/blob/v0.1.0/priv/markov.lua
import random
from typing import TextIO

# Anything the markov chainer considers punctuation
SANITIZATION_PUNCTUATION = frozenset(ch for ch in "!.?")
# Anything the markov chainer considers lowercase letters
SANITIZATION_LETTERS = frozenset(ch for ch in "abcdefgihjklmnopqrstuvwxyz")


class Chainer:
    """
    A Markov chain based text generator.
    """

    def __init__(self):
        """
        Initializes an empty Markov chain text generator.
        """
        self.starters: set[str] = set()
        self.mids: dict[str, set[str]] = {}
        self.ends: dict[str, set[str]] = {}

    def parse(self, file: TextIO) -> None:
        """
        Load a text file into the Markov chain text generator.
        """
        # Read all of the file, what if we have a newline?
        split_file: list[str] = (" ".join(file.readlines())).split(" ")

        # Start off with no words
        previous_word = None

        # Iterate each space-separated word
        for mixed_word in split_file:
            # Easier to do lowercase handling, we're using frozensets
            word = mixed_word.lower()

            chars = [ch for ch in word if ch in SANITIZATION_LETTERS]
            stripped_word = "".join(chars)

            if stripped_word == "":
                # There is nothing in here so don't bother
                continue

            sentence_ends = word[-1] in SANITIZATION_PUNCTUATION

            # Check if the previous word is nothing
            if previous_word == None:
                self.starters.add(stripped_word)

                # We have a previous word
                previous_word = stripped_word

            # Check if the sentence ends, or if we're in the middle of a
            # sentence
            if sentence_ends:
                # The sentence ended
                # Check if we have that punctuation present
                if stripped_word not in self.ends:
                    # We don't have it, add an end set to this entry
                    self.ends[stripped_word] = set()

                # Add the punctuation to that set anyways
                self.ends[stripped_word].add(word[-1])

                # The sentence has ended
                previous_word = None
            elif previous_word != None:
                # We are in the middle of a sentence
                if previous_word not in self.mids:
                    # We don't have that sentence middle
                    self.mids[previous_word] = set()

                # Add the current word to the set anyways
                self.mids[previous_word].add(stripped_word)

                # We have a previous word
                previous_word = stripped_word

    def run(self, minimum_hint: int, maximum_words: int) -> str:
        """
        Runs the text generator with a hinted minimum word count and a maximum
        word count.
        """
        # The chained sentence
        sentence: list[str] = [random.choice(tuple(self.starters))]

        # A loop to iterate the sentence words
        while len(sentence) < maximum_words:
            # Get our last word
            last_word: str = sentence[-1]
            # Is the last word able to end this chain?
            if last_word in self.ends:
                if (
                    len(sentence) > minimum_hint
                    or last_word not in self.mids[last_word]
                ):
                    # We can end here because we're over our hint, or we must
                    # end because there's no mids left
                    sentence[-1] += random.choice(tuple(self.ends[last_word]))
                    return " ".join(sentence).capitalize()
                else:
                    # We can continue
                    sentence.append(random.choice(tuple(self.mids[last_word])))
            else:
                # Append a random word to the sentence
                sentence.append(random.choice(tuple(self.mids[last_word])))

        # We end with ellipsis to signify our sentence went over the
        # length limit without ending
        sentence[-1] += "..."
        return " ".join(sentence).capitalize()
