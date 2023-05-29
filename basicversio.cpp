#include <bits/stdc++.h>
using namespace std; 

class StockOrder {
    public:
        int orderId = -1;
        string symbol;
        string side; 
        float price; 
        int volume;
        int timestamp;     
    
    StockOrder(int orderId, string symbol, string side, float price, int volume, int timestamp) {
        this->orderId = orderId; 
        this->symbol = symbol;
        this->side = side; 
        this->price = price; 
        this->volume = volume; 
        this->timestamp = timestamp;
    } 
    
    StockOrder() {}
};

class MatchedOrders {
    public: 
        string symbol; 
        float price; 
        int volume; 
        int aggressive_order_id; 
        int passive_order_id;
      
    MatchedOrders(string symbol, float price, int volume, int aggressive_order_id, int passive_order_id) {
        this->symbol = symbol;
        this->price = price; 
        this->volume = volume; 
        this->aggressive_order_id = aggressive_order_id; 
        this->passive_order_id = passive_order_id;
    }
    MatchedOrders() {}
};

struct customComparator {
    bool operator() (StockOrder a, StockOrder b) const {
        if (a.side == "BUY") {
            if (a.price > b.price) return true; 
            else if (a.price < b.price) return false; 
            else if (a.timestamp < b.timestamp) return true; 
            else return false; 
        } else {
            if (a.price < b.price) return true; 
            else if (a.price > b.price) return false; 
            else if (a.timestamp < b.timestamp) return true; 
            else return false; 
        } 
    }
};

string convertFloatToString(float x) {
    stringstream s;
    s<<x; 
    string result=s.str(); 
    return result; 
}

float convertToFloat(const std::string& priceString) {
    size_t dotPos = priceString.find('.');
    if (dotPos == std::string::npos) {
        // No decimal point found, assuming whole number
        return std::stof(priceString);
    }

    size_t digitsAfterDot = priceString.length() - dotPos - 1;
    if (digitsAfterDot > 4) {
        // More than 4 digits after the decimal point
        cout << "String input error, more than 4 number behind the decimal"; 
    }

    return std::stof(priceString);
}

vector<string> splitString(string s){
    for(int i = 0; i < s.size(); ++i){
        if(s[i] == ',') s[i] = ' ';
    }
    istringstream is(s);
    vector<string> result;
    string x;
    while(is >> x) result.push_back(x);
    return result;   
}

void findMatch(StockOrder* curOrder, vector<string>& result, unordered_map<int, StockOrder>& orderLookUp, map<string, set<StockOrder, customComparator>>& symbolLookUp, set<string>& allSymbols) {
    string symbol = curOrder->symbol; 
    string side = curOrder->side;
    int orderId = curOrder->orderId; 
    string key = symbol + side;
    allSymbols.insert(symbol);
    orderLookUp[orderId] = *curOrder; 
    symbolLookUp[key].insert(*curOrder);
    string oppositeSide;  
    if (side == "BUY") {
        oppositeSide = "SELL"; 
    } else {
        oppositeSide = "BUY";
    }  
    string oppositeKey = symbol + oppositeSide;
    vector<MatchedOrders> vecMatchedOrders;
    while (true) {
        if (symbolLookUp.find(oppositeKey) == symbolLookUp.end()) {
            break; 
        }  
        StockOrder potentialMatch = *symbolLookUp[oppositeKey].begin();
 
        if (symbolLookUp[key].size() == 1) {
            symbolLookUp.erase(key);
        } else {
            symbolLookUp[key].erase(*curOrder); 
        }
        if (symbolLookUp[oppositeKey].size() == 1) {
            symbolLookUp.erase(oppositeKey);
        } else {
            symbolLookUp[oppositeKey].erase(potentialMatch); 
        }
        if (curOrder->side == "SELL" && potentialMatch.price >= curOrder->price) {
            int tmp = min(potentialMatch.volume, curOrder->volume); 
            curOrder->volume -= tmp; 
            potentialMatch.volume -= tmp; 
            MatchedOrders matches(symbol, potentialMatch.price, tmp, curOrder->orderId, potentialMatch.orderId); 
            vecMatchedOrders.push_back(matches);
        } else if (curOrder->side == "BUY" && potentialMatch.price <= curOrder->price) {
            int tmp = min(potentialMatch.volume, curOrder->volume); 
            curOrder->volume -= tmp; 
            potentialMatch.volume -= tmp; 
            MatchedOrders matches(symbol, potentialMatch.price, tmp, curOrder->orderId, potentialMatch.orderId); 
            vecMatchedOrders.push_back(matches);
        } else {
            orderLookUp[orderId] = *curOrder; 
            symbolLookUp[key].insert(*curOrder); 
            orderLookUp[potentialMatch.orderId] = potentialMatch; 
            symbolLookUp[oppositeKey].insert(potentialMatch);
            break;
        }  

        if (potentialMatch.volume != 0) {
            orderLookUp[potentialMatch.orderId] = potentialMatch; 
            symbolLookUp[oppositeKey].insert(potentialMatch);
        }

        if (curOrder->volume != 0) {
            orderLookUp[orderId] = *curOrder; 
            symbolLookUp[key].insert(*curOrder); 
        } else {
            break; 
        }
    }
    for (MatchedOrders& matchedOrders : vecMatchedOrders) {
        result.push_back(matchedOrders.symbol + "," + convertFloatToString(matchedOrders.price) + "," + to_string(matchedOrders.volume) + "," +  to_string(matchedOrders.aggressive_order_id) + "," + to_string(matchedOrders.passive_order_id));
    } 
    return;
}


void processInsertQuery(vector<string> command, vector<string>& result, unordered_map<int, StockOrder>& orderLookUp, map<string, set<StockOrder, customComparator>>& symbolLookUp, set<string>& allSymbols) {
    int orderId = stoi(command[1]); 
    string symbol = command[2];
    string side = command[3];
    float price = convertToFloat(command[4]); 
    int volume = stoi(command[5]);
    int timestamp = stoi(command[6]); 
    StockOrder curOrder(orderId, symbol, side, price, volume, timestamp);
    findMatch(&curOrder, result, orderLookUp, symbolLookUp, allSymbols); 
    return;    
}
  
void processAmendQuery(vector<string> command, vector<string>& result, unordered_map<int, StockOrder>& orderLookUp, map<string, set<StockOrder, customComparator>>& symbolLookUp, set<string>& allSymbols) {
    int orderId = stoi(command[1]); 
    float price = convertToFloat(command[2]); 
    int volume = stoi(command[3]);
    int timestamp = stoi(command[4]);    
    StockOrder curOrder; 
    if (orderLookUp.find(orderId) != orderLookUp.end()) {
        curOrder = orderLookUp[orderId]; 
    } else {
        cout << "Invalid amend request";
    }  
    
    //Nothing changes
    if (curOrder.volume == volume && curOrder.price == price) {
        return; 
    }
    //If it is only volume change then only needs to change the volume 
    // This means that the prices changes 
    //If the price change then, revaluate and added, order in again again
    //Delete the order from the table 
    string key = curOrder.symbol + curOrder.side; 
    if (symbolLookUp[key].size() == 1) {
        symbolLookUp.erase(key);
    } else {
        symbolLookUp[key].erase(curOrder); 
    }    
    orderLookUp.erase(orderId); 

    curOrder.price = price; 
    if (curOrder.volume <= volume) {
        curOrder.timestamp = timestamp;
    }
    curOrder.volume = volume; 
     
    orderLookUp[orderId] = curOrder; 
    symbolLookUp[key].insert(curOrder); 
    findMatch(&curOrder, result, orderLookUp, symbolLookUp, allSymbols); 

}  

void processPullQuery(vector<string> command, unordered_map<int, StockOrder>& orderLookUp, map<string, set<StockOrder, customComparator>>& symbolLookUp, set<string>& allSymbols) {
    int orderId = stoi(command[1]);
    if (orderLookUp.find(orderId) != orderLookUp.end()) {
        StockOrder curOrder = orderLookUp[orderId]; 
        string symbol = curOrder.symbol + curOrder.side; 
        symbolLookUp[symbol].erase(curOrder);
        orderLookUp.erase(orderId); 
    } else {
        cout << "Invalid pull request";
    }
}



vector<string> run(vector<string> const& input) {
    vector<string> result; 
    unordered_map<int, StockOrder> orderLookUp; 
    map<string, set<StockOrder, customComparator>> symbolLookUp; 
    set<string> allSymbols;
    for (int i = 0; i < input.size(); i++) {
        vector<string> command = splitString(input[i]);
        //Add the timestamp;
        command.push_back(to_string(i));
        if (command[0] == "INSERT") {
            processInsertQuery(command, result, orderLookUp, symbolLookUp, allSymbols); 
        } else if (command[0] == "AMEND") {
            processAmendQuery(command, result, orderLookUp, symbolLookUp, allSymbols);  
        } else if (command[0] == "PULL") {
            processPullQuery(command, orderLookUp, symbolLookUp, allSymbols);
        }
    } 

    for (const string& symbol : allSymbols) {
        string buyKey = symbol + "BUY"; 
        string sellKey = symbol + "SELL"; 
        map<float, int, std::greater<float>> buy; 
        map<float, int> sell;
        if (symbolLookUp.find(buyKey) != symbolLookUp.end() || symbolLookUp.find(sellKey) != symbolLookUp.end()) {
            result.push_back("===" + symbol + "===");
        }
        if (symbolLookUp.find(buyKey) != symbolLookUp.end()) {
            for (auto it = symbolLookUp[buyKey].begin(); it != symbolLookUp[buyKey].end(); it++) {
                buy[it->price] += it->volume; 
            }
        }
        if (symbolLookUp.find(sellKey) != symbolLookUp.end()) {
            for (auto it = symbolLookUp[sellKey].begin(); it != symbolLookUp[sellKey].end(); it++) {
                sell[it->price] += it->volume; 
            }
        }
        auto it1 = buy.begin(); 
        auto it2 = sell.begin(); 
        while (it1 != buy.end() || it2 != sell.end()) {
            if (it1 != buy.end() && it2 != sell.end()) {
               result.push_back(convertFloatToString(it1->first) + "," + to_string(it1->second) + "," + convertFloatToString(it2->first) + "," + to_string(it2->second));
               it1++; 
               it2++; 
            } else if (it1 != buy.end()) {
                result.push_back(convertFloatToString(it1->first) + "," + to_string(it1->second) + ",,");
                it1++; 
            } else {
                result.push_back( ",," + convertFloatToString(it2->first) + "," + to_string(it2->second));
                it2++;  
            }
        }

    }   
    return result;
}


int main() {
    int line = 0;
    cin >> line;  
    vector<string> command; 
    for (int i = 0; i < line; i++) {
        string tmp; 
        cin >> tmp; 
        command.push_back(tmp); 
    }
    vector<string> tmp1 = run(command);
    for (string line : tmp1) {
        cout << line << endl;
    } 
    return 0; 
}