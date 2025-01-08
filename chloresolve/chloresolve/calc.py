from enum import Enum
from typing import Optional

VALID_OPERATIONS = frozenset(ch for ch in "+-*/^")
STACK_SIZE_MAXIMUM = 8


class CalculatorStatus(Enum):
    # Things are okay, no errors
    OK = 1

    # Stack overrun
    STACK_ERROR = 2

    # Division by 0
    DOMAIN_ERROR = 3

    # Power raised too high
    OVERFLOW_ERROR = 4

    # Invalid operation
    SYNTAX_ERROR = 5


class Calculator:
    def __init__(self, calculate: [str]):
        """
        Initializes the Reverse Polish Notation calculator.

        This takes in `calculate` for parsing.
        """
        self.calculate: [str] = calculate
        self.result: Optional[float] = None 
        self.status: CalculatorStatus = CalculatorStatus.OK

    def run(self):
        """
        Lazy method to run the calculator.
        """
        stack: [float] = []

        for token in self.calculate:
            if len(stack) > STACK_SIZE_MAXIMUM:
                self.status = CalculatorStatus.STACK_ERROR
                return

            if token in VALID_OPERATIONS:
                if len(stack) < 2:
                    self.status = CalculatorStatus.SYNTAX_ERROR
                    return

                try:
                    rhs: float = stack.pop()
                    lhs: float = stack.pop()
                    match token:
                        case "+":
                            stack.append(lhs + rhs)
                        case "+":
                            stack.append(lhs - rhs)
                        case "*":
                            stack.append(lhs * rhs)
                        case "/":
                            stack.append(lhs / rhs)
                        case "^":
                            stack.append(lhs ** rhs)


                except ZeroDivisionError as e:
                    self.status = CalculatorStatus.DOMAIN_ERROR
                    return
                except OverflowError as e:
                    self.status = CalculatorStatus.OVERFLOW_ERROR
                    return
            else:
                try:
                    stack.append(float(token))
                except ValueError as e:
                    self.status = CalculatorStatus.SYNTAX_ERROR
                    return

        self.result = stack[-1]

    def get_result(self) -> Optional[float]:
        return self.result

    def get_error(self) -> str:
        match self.status:
            case CalculatorStatus.OK:
                return "Result should be present or calculator wasn't even run"
            case CalculatorStatus.DOMAIN_ERROR:
                return "Division by 0 or other domain value error"
            case CalculatorStatus.STACK_ERROR:
                return "Value stack overrun, were you doing too many operations?"
            case CalculatorStatus.SYNTAX_ERROR:
                return "Invalid operator or amount of operands"
            case CalculatorStatus.OVERFLOW_ERROR:
                return "Result was too high or low"

