#include <bits/stdc++.h>
using namespace std; 

vector<string> res; 
class StockOrder {
    public:
        int orderId = -1;
        string symbol;
        string side; 
        float price; 
        int volume;
        int timestamp;  
        StockOrder *nextOrder; 
        StockOrder *prevOrder;   
    
    StockOrder(int orderId, string symbol, string side, float price, int volume, int timestamp) {
        this->orderId = orderId; 
        this->symbol = symbol;
        this->side = side; 
        this->price = price; 
        this->volume = volume; 
        this->timestamp = timestamp;
        this->nextOrder = NULL; 
        this->prevOrder = NULL; 
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

class Limit {
    public: 
        float limitPrice; 
        int totalVolume;
        string side;
        int size;  
        StockOrder* headOrder; 
        StockOrder* tailOrder;

    Limit(float limitPrice, string side, int totalVolume, StockOrder* headOrder, StockOrder* tailOrder) {
        this->limitPrice = limitPrice; 
        this->totalVolume = 0; 
        this->headOrder = headOrder; 
        this->tailOrder = tailOrder; 
        this->size = 0; 
    }

    Limit() {}
}; 

struct customComparator {
    bool operator() (Limit a, Limit b) const {
        if (a.side == "BUY") {
            return (a.limitPrice > b.limitPrice); 
        } else {
            return (a.limitPrice < b.limitPrice);
        } 
    }
};

string convertFloatToString(float x) {
    stringstream s;
    s<<x; 
    string result=s.str(); 
    return result; 
}

unordered_map<int, StockOrder> orderLookUp; 
//This string will be SymBol + BUY/SELL + price Limit
unordered_map<string, Limit> limitLookUp; 
//This string will be SymBol
unordered_map<string, set<float>> limitSetSellLookUp;
unordered_map<string, set<float, greater<float>>> limitSetBuyLookUp;

set<string> allSymbols;

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

void removeOrder(StockOrder* curOrder) {
    int orderId = curOrder->orderId;
    orderLookUp.erase(orderId); 
    string keyLimitLookUp = curOrder->symbol + curOrder->side + convertFloatToString(curOrder->price);
    Limit *curLimit = &limitLookUp[keyLimitLookUp];

    //Delete an element from a linked list. 
    curLimit->totalVolume -= curOrder->volume;
    curLimit->size -= 1; 
    if (curOrder->prevOrder == NULL) {
        curLimit->headOrder = curOrder->nextOrder; 
    } else {
        curOrder->prevOrder->nextOrder = curOrder->nextOrder;
        if (curOrder->nextOrder != NULL) {
            curOrder->nextOrder->prevOrder = curOrder->prevOrder; 
        }
    }
    if (curLimit->size == 0) {
        limitLookUp.erase(keyLimitLookUp);
        if (curOrder->side == "BUY") {
            limitSetBuyLookUp[curOrder->symbol].erase(curLimit->limitPrice);
        } else {
            limitSetSellLookUp[curOrder->symbol].erase(curLimit->limitPrice);
        }
    }
}

void addOrder(StockOrder* curOrder) {
    string symbol = curOrder->symbol;
    string side = curOrder->side; 
    int orderId = curOrder->orderId; 
    int volume = curOrder->volume; 
    float price = curOrder->price;
    allSymbols.insert(symbol);
    orderLookUp[orderId] = *curOrder;
    string keyLimitLookUp = symbol + side + convertFloatToString(price); 
    if (limitLookUp.find(keyLimitLookUp) == limitLookUp.end()) {
        Limit curLimit(price, side, volume, &orderLookUp[orderId], &orderLookUp[orderId]); 
        limitLookUp[keyLimitLookUp] = curLimit; 
    } else {
        //Add the order to the beginning of the linked list 
        limitLookUp[keyLimitLookUp].headOrder->prevOrder = &orderLookUp[orderId];
        curOrder->nextOrder = limitLookUp[keyLimitLookUp].headOrder;
        limitLookUp[keyLimitLookUp].headOrder = curOrder;
    }
    limitLookUp[keyLimitLookUp].size += 1; 
    limitLookUp[keyLimitLookUp].totalVolume += volume;
    //Update the map 
    if (side == "BUY") {
        limitSetBuyLookUp[symbol].insert(price);
    } else {
        limitSetSellLookUp[symbol].insert(price);
    } 
} 

void matchOrder(StockOrder* curOrder) {
    int orderId = curOrder->orderId;
    string symbol = curOrder->symbol; 
    string side = curOrder->side; 
    float price = curOrder->price;
    int volume = curOrder->volume; 
    int timestamp = curOrder->timestamp; 
    //Match the current order with the order from the opposite side 
    string keyLimitLookUp = symbol+side+convertFloatToString(price); 
    string oppositeSide = (side == "BUY") ? "SELL" : "BUY";
    vector<MatchedOrders> vecMatchedOrders;  
    while (true) {
        //No opposite price
        if ((side == "BUY" && limitSetSellLookUp[symbol].size() == 0) || (side == "SELL" && limitSetBuyLookUp[symbol].size() == 0)) {
            break; 
        } 
        float potentialMatchPrice;
        if (side == "BUY") {
            potentialMatchPrice = *limitSetSellLookUp[symbol].begin();
        } else {
            potentialMatchPrice = *limitSetBuyLookUp[symbol].begin();
        }
        string oppositekeyLimitLookUp = symbol + oppositeSide + convertFloatToString(potentialMatchPrice); 
        Limit* potentialMatchLimit = &limitLookUp[oppositekeyLimitLookUp]; 
        StockOrder* potentialMatchOrder = potentialMatchLimit->tailOrder; 
        if (curOrder->side == "SELL" && potentialMatchOrder->price >= curOrder->price) {
            int tmp = min(potentialMatchOrder->volume, curOrder->volume); 
            curOrder->volume -= tmp; 
            potentialMatchOrder->volume -= tmp; 
            potentialMatchLimit->totalVolume -= tmp; 
            MatchedOrders matches(symbol, potentialMatchOrder->price, tmp, curOrder->orderId, potentialMatchOrder->orderId); 
            vecMatchedOrders.push_back(matches);
        } else if (curOrder->side == "BUY" && potentialMatchOrder->price <= curOrder->price) {
            int tmp = min(potentialMatchOrder->volume, curOrder->volume); 
            curOrder->volume -= tmp; 
            potentialMatchOrder->volume -= tmp; 
            potentialMatchLimit->totalVolume -= tmp; 
            MatchedOrders matches(symbol, potentialMatchOrder->price, tmp, curOrder->orderId, potentialMatchOrder->orderId); 
            vecMatchedOrders.push_back(matches);
        } else { 
            //There is no match anymore, so break; 
            break;
        }  
        if (potentialMatchOrder->volume == 0) {
            //Hand in the remove all in here
            removeOrder(potentialMatchOrder);
        } 
        
        //There is a match, so now we need to remove it from the order from the order book. 
        if (curOrder->volume == 0) {
            break;
        }
        
    }
    if (curOrder->volume != 0) {
        addOrder(curOrder); 
    }
    for (MatchedOrders& matchedOrders : vecMatchedOrders) {
        res.push_back(matchedOrders.symbol + "," + convertFloatToString(matchedOrders.price) + "," + to_string(matchedOrders.volume) + "," +  to_string(matchedOrders.aggressive_order_id) + "," + to_string(matchedOrders.passive_order_id));
    }
    return; 
}


void processInsertQuery(vector<string> command) {
    int orderId = stoi(command[1]); 
    string symbol = command[2];
    string side = command[3];
    float price = convertToFloat(command[4]); 
    int volume = stoi(command[5]);
    int timestamp = stoi(command[6]); 
    //Current order
    StockOrder curOrder(orderId, symbol, side, price, volume, timestamp);

    //I have not added the order yet. 
    //It will added in the matchOrder
    matchOrder(&curOrder); 
    return;    
}
  
void processAmendQuery(vector<string> command) {
    
    return; 
}  

void processPullQuery(vector<string> command) {
    int orderId = stoi(command[1]);
    if (orderLookUp.find(orderId) != orderLookUp.end()) {
        StockOrder curOrder = orderLookUp[orderId]; 
        removeOrder(&curOrder);
    } else {
        cout << "Invalid pull request";
    }
}


void outPutPerSymbol() {
    for (const string& symbol : allSymbols) {
        if (limitSetBuyLookUp[symbol].size() != 0 || limitSetSellLookUp[symbol].size() != 0) {
            res.push_back("===" + symbol + "===");
        } 
        auto it1 = limitSetBuyLookUp[symbol].begin(); 
        auto it2 = limitSetSellLookUp[symbol].begin(); 
        while (it1 != limitSetBuyLookUp[symbol].end() || it2 != limitSetSellLookUp[symbol].end()) {
            if (it1 != limitSetBuyLookUp[symbol].end() && it2 != limitSetSellLookUp[symbol].end()) {
               res.push_back(convertFloatToString(*it1) + "," + to_string(limitLookUp[symbol + "BUY" + convertFloatToString(*it1)].totalVolume) + "," + convertFloatToString(*it2) + "," + to_string(limitLookUp[symbol + "SELL" + convertFloatToString(*it2)].totalVolume));
               it1++; 
               it2++; 
            } else if (it1 != limitSetBuyLookUp[symbol].end()) {
                res.push_back(convertFloatToString(*it1) + "," + to_string(limitLookUp[symbol + "BUY" + convertFloatToString(*it1)].totalVolume) + ",,");
                it1++;
            } else {
                res.push_back( ",," + convertFloatToString(*it2) + "," + to_string(limitLookUp[symbol + "SELL" + convertFloatToString(*it2)].totalVolume));
                it2++;  
            }
        }
    }
}

vector<string> run(vector<string> const& input) {
    for (int i = 0; i < input.size(); i++) {
        vector<string> command = splitString(input[i]);
        //Add the timestamp; 
        command.push_back(to_string(i));
        if (command[0] == "INSERT") {
            processInsertQuery(command); 
        } else if (command[0] == "AMEND") {
            processAmendQuery(command);  
        } else if (command[0] == "PULL") {
            processPullQuery(command);
        }
    } 
    outPutPerSymbol();
    return res;
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