
import java.util.Stack;

public class Stack {
    private Stack<Integer> states = new Stack<>();
    private Stack<String> symbols = new Stack<>();

    public void push(int state, String symbol) {
        states.push(state);
        symbols.push(symbol);
    }

    public void pop(int n) {
        for (int i = 0; i < n; i++) {
            if (!states.isEmpty()) states.pop();
            if (!symbols.isEmpty()) symbols.pop();
        }
    }
}
