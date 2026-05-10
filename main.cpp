
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <limits>
#include <algorithm>

using namespace std;

// ─────────────────────────────────────────────
//  UTILITY HELPERS
// ─────────────────────────────────────────────
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << "\n  Press ENTER to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void printLine(char ch = '-', int width = 62) {
    cout << "  " << string(width, ch) << "\n";
}

void printHeader(const string& title) {
    clearScreen();
    cout << "\n";
    printLine('=');
    cout << setw(38 + title.size() / 2) << title << "\n";
    printLine('=');
    cout << "\n";
}

string getCurrentTime() {
    time_t now = time(0);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

// ─────────────────────────────────────────────
//  CLASS: Transaction
// ─────────────────────────────────────────────
class Transaction {
public:
    long long accountNo;
    char      type[15];     // "DEPOSIT", "WITHDRAWAL", "CREATED"
    double    amount;
    double    balanceAfter;
    char      timestamp[20];

    Transaction() {}
    Transaction(long long acc, const string& t, double amt, double bal) {
        accountNo = acc;
        t.copy(type, 14); type[min((int)t.size(), 14)] = '\0';
        amount      = amt;
        balanceAfter = bal;
        string ts = getCurrentTime();
        ts.copy(timestamp, 19); timestamp[19] = '\0';
    }

    void save() const {
        ofstream fout("transactions.dat", ios::binary | ios::app);
        fout.write(reinterpret_cast<const char*>(this), sizeof(*this));
        fout.close();
    }

    static vector<Transaction> getByAccount(long long accNo) {
        vector<Transaction> list;
        ifstream fin("transactions.dat", ios::binary);
        if (!fin) return list;
        Transaction t;
        while (fin.read(reinterpret_cast<char*>(&t), sizeof(t)))
            if (t.accountNo == accNo) list.push_back(t);
        fin.close();
        return list;
    }
};

// ─────────────────────────────────────────────
//  BASE CLASS: Account
// ─────────────────────────────────────────────
class Account {
protected:
    long long accountNo;
    char      holderName[50];
    char      phone[15];
    double    balance;
    char      accountType[15];   // "SAVINGS" or "CURRENT"
    char      pin[5];            // 4-digit PIN

public:
    // ── Constructors ──
    Account() {}

    Account(long long no, const string& name, const string& ph,
            double bal, const string& type, const string& p) {
        accountNo = no;
        name.copy(holderName, 49); holderName[min((int)name.size(), 49)] = '\0';
        ph.copy(phone, 14);        phone[min((int)ph.size(), 14)]         = '\0';
        balance = bal;
        type.copy(accountType, 14); accountType[min((int)type.size(), 14)] = '\0';
        p.copy(pin, 4);            pin[4] = '\0';
    }

    // ── Getters ──
    long long   getAccountNo()   const { return accountNo; }
    string      getName()        const { return string(holderName); }
    string      getPhone()       const { return string(phone); }
    double      getBalance()     const { return balance; }
    string      getAccountType() const { return string(accountType); }
    string      getPin()         const { return string(pin); }

    // ── Core Banking Operations ──
    virtual bool deposit(double amount) {
        if (amount <= 0) return false;
        balance += amount;
        Transaction(accountNo, "DEPOSIT", amount, balance).save();
        return true;
    }

    virtual bool withdraw(double amount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        Transaction(accountNo, "WITHDRAWAL", amount, balance).save();
        return true;
    }

    // ── Display ──
    virtual void displayDetails() const {
        printLine();
        cout << "  Account No   : " << accountNo           << "\n"
             << "  Holder Name  : " << holderName          << "\n"
             << "  Phone        : " << phone                << "\n"
             << "  Account Type : " << accountType          << "\n"
             << "  Balance      : Rs. " << fixed << setprecision(2) << balance << "\n";
        printLine();
    }

    bool verifyPin(const string& p) const { return string(pin) == p; }

    virtual ~Account() {}
};

// ─────────────────────────────────────────────
//  DERIVED CLASS: SavingsAccount
// ─────────────────────────────────────────────
class SavingsAccount : public Account {
    double interestRate;  // annual %
public:
    SavingsAccount() : Account() { interestRate = 4.0; }

    SavingsAccount(long long no, const string& name, const string& ph,
                   double bal, const string& p)
        : Account(no, name, ph, bal, "SAVINGS", p), interestRate(4.0) {}

    // Savings: minimum balance Rs. 500
    bool withdraw(double amount) override {
        if (amount <= 0)                  return false;
        if ((balance - amount) < 500.0) {
            cout << "  [!] Minimum balance of Rs.500 must be maintained.\n";
            return false;
        }
        balance -= amount;
        Transaction(accountNo, "WITHDRAWAL", amount, balance).save();
        return true;
    }

    void displayDetails() const override {
        Account::displayDetails();
        cout << "  Interest Rate: " << interestRate << "% per annum\n"
             << "  Min Balance  : Rs. 500.00\n";
        printLine();
    }

    double getInterestRate() const { return interestRate; }
};

// ─────────────────────────────────────────────
//  DERIVED CLASS: CurrentAccount
// ─────────────────────────────────────────────
class CurrentAccount : public Account {
    double overdraftLimit;
public:
    CurrentAccount() : Account() { overdraftLimit = 10000.0; }

    CurrentAccount(long long no, const string& name, const string& ph,
                   double bal, const string& p)
        : Account(no, name, ph, bal, "CURRENT", p), overdraftLimit(10000.0) {}

    // Current: allows overdraft up to limit
    bool withdraw(double amount) override {
        if (amount <= 0) return false;
        if ((balance - amount) < -overdraftLimit) {
            cout << "  [!] Overdraft limit of Rs." << overdraftLimit << " exceeded.\n";
            return false;
        }
        balance -= amount;
        Transaction(accountNo, "WITHDRAWAL", amount, balance).save();
        return true;
    }

    void displayDetails() const override {
        Account::displayDetails();
        cout << "  Overdraft Limit: Rs. " << fixed << setprecision(2) << overdraftLimit << "\n";
        printLine();
    }
};

// ─────────────────────────────────────────────
//  CLASS: Bank  (manages all accounts)
// ─────────────────────────────────────────────
class Bank {
    const string FILE_NAME = "accounts.dat";

    // Raw struct stored in file
    struct AccountRecord {
        long long accountNo;
        char      holderName[50];
        char      phone[15];
        double    balance;
        char      accountType[15];
        char      pin[5];
    };

    // ── File helpers ──
    vector<AccountRecord> loadAll() const {
        vector<AccountRecord> list;
        ifstream fin(FILE_NAME, ios::binary);
        if (!fin) return list;
        AccountRecord r;
        while (fin.read(reinterpret_cast<char*>(&r), sizeof(r)))
            list.push_back(r);
        fin.close();
        return list;
    }

    void saveAll(const vector<AccountRecord>& list) const {
        ofstream fout(FILE_NAME, ios::binary | ios::trunc);
        for (const auto& r : list)
            fout.write(reinterpret_cast<const char*>(&r), sizeof(r));
        fout.close();
    }

    long long generateAccountNo() const {
        auto list = loadAll();
        long long base = 1000000000LL;
        return list.empty() ? base : list.back().accountNo + 1;
    }

    AccountRecord toRecord(const Account& a) const {
        AccountRecord r;
        r.accountNo = a.getAccountNo();
        a.getName().copy(r.holderName, 49);  r.holderName[min((int)a.getName().size(),49)] = '\0';
        a.getPhone().copy(r.phone, 14);      r.phone[min((int)a.getPhone().size(),14)]      = '\0';
        r.balance = a.getBalance();
        a.getAccountType().copy(r.accountType, 14); r.accountType[min((int)a.getAccountType().size(),14)] = '\0';
        a.getPin().copy(r.pin, 4); r.pin[4] = '\0';
        return r;
    }

public:
    // ── 1. CREATE ACCOUNT ──
    void createAccount() {
        printHeader("  CREATE NEW ACCOUNT");

        string name, phone, pin, typeChoice;
        double initialDeposit;
        int    accTypeNum;

        cout << "  Select Account Type:\n"
             << "    1. Savings Account  (Min Balance: Rs.500, Interest: 4% p.a.)\n"
             << "    2. Current Account  (Overdraft Limit: Rs.10,000)\n\n"
             << "  Enter choice (1/2) : ";
        cin >> accTypeNum; cin.ignore();

        if (accTypeNum != 1 && accTypeNum != 2) {
            cout << "  [!] Invalid account type.\n";
            pauseScreen(); return;
        }

        cout << "  Enter Full Name     : "; getline(cin, name);
        cout << "  Enter Phone Number  : "; getline(cin, phone);

        // PIN setup
        while (true) {
            cout << "  Set 4-digit PIN     : "; getline(cin, pin);
            if (pin.size() == 4 && all_of(pin.begin(), pin.end(), ::isdigit)) break;
            cout << "  [!] PIN must be exactly 4 digits.\n";
        }

        double minDeposit = (accTypeNum == 1) ? 500.0 : 0.0;
        while (true) {
            cout << "  Initial Deposit (Min Rs." << fixed << setprecision(0)
                 << minDeposit << ") : Rs. ";
            if (cin >> initialDeposit && initialDeposit >= minDeposit) break;
            cout << "  [!] Amount must be at least Rs." << minDeposit << "\n";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore();

        long long newNo = generateAccountNo();

        AccountRecord r;
        r.accountNo = newNo;
        name.copy(r.holderName, 49);  r.holderName[min((int)name.size(),49)]  = '\0';
        phone.copy(r.phone, 14);      r.phone[min((int)phone.size(),14)]       = '\0';
        r.balance = initialDeposit;
        string atype = (accTypeNum == 1) ? "SAVINGS" : "CURRENT";
        atype.copy(r.accountType, 14); r.accountType[min((int)atype.size(),14)] = '\0';
        pin.copy(r.pin, 4); r.pin[4] = '\0';

        auto list = loadAll();
        list.push_back(r);
        saveAll(list);

        // Log creation transaction
        Transaction(newNo, "CREATED", initialDeposit, initialDeposit).save();

        cout << "\n";
        printLine('*');
        cout << "  [✓] Account Created Successfully!\n"
             << "  Your Account Number : " << newNo << "\n"
             << "  Please keep it safe.\n";
        printLine('*');
        pauseScreen();
    }

    // ── Helper: get + verify account ──
    bool getVerifiedRecord(AccountRecord& found) {
        long long accNo;
        string    pin;

        cout << "  Enter Account Number : "; cin >> accNo; cin.ignore();
        cout << "  Enter 4-digit PIN    : "; getline(cin, pin);

        auto list = loadAll();
        for (auto& r : list) {
            if (r.accountNo == accNo) {
                if (string(r.pin) == pin) { found = r; return true; }
                cout << "\n  [!] Incorrect PIN.\n";
                pauseScreen(); return false;
            }
        }
        cout << "\n  [!] Account not found.\n";
        pauseScreen(); return false;
    }

    void updateRecord(const AccountRecord& updated) {
        auto list = loadAll();
        for (auto& r : list)
            if (r.accountNo == updated.accountNo) { r = updated; break; }
        saveAll(list);
    }

    // ── 2. DEPOSIT ──
    void deposit() {
        printHeader("  DEPOSIT MONEY");

        AccountRecord r;
        if (!getVerifiedRecord(r)) return;

        double amount;
        cout << "\n  Current Balance : Rs. " << fixed << setprecision(2) << r.balance << "\n";
        cout << "  Enter Deposit Amount : Rs. ";
        if (!(cin >> amount) || amount <= 0) {
            cout << "  [!] Invalid amount.\n"; cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pauseScreen(); return;
        }
        cin.ignore();

        r.balance += amount;
        Transaction(r.accountNo, "DEPOSIT", amount, r.balance).save();
        updateRecord(r);

        cout << "\n";
        printLine('*');
        cout << "  [✓] Rs. " << fixed << setprecision(2) << amount << " deposited successfully!\n"
             << "  New Balance : Rs. " << r.balance << "\n";
        printLine('*');
        pauseScreen();
    }

    // ── 3. WITHDRAW ──
    void withdraw() {
        printHeader("  WITHDRAW MONEY");

        AccountRecord r;
        if (!getVerifiedRecord(r)) return;

        double amount;
        cout << "\n  Current Balance : Rs. " << fixed << setprecision(2) << r.balance << "\n";
        cout << "  Enter Withdrawal Amount : Rs. ";
        if (!(cin >> amount) || amount <= 0) {
            cout << "  [!] Invalid amount.\n"; cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pauseScreen(); return;
        }
        cin.ignore();

        bool isSavings = (string(r.accountType) == "SAVINGS");
        double minBal  = isSavings ? 500.0 : -10000.0;

        if ((r.balance - amount) < minBal) {
            if (isSavings)
                cout << "\n  [!] Insufficient funds. Minimum balance of Rs.500 must be maintained.\n";
            else
                cout << "\n  [!] Overdraft limit of Rs.10,000 exceeded.\n";
            pauseScreen(); return;
        }

        r.balance -= amount;
        Transaction(r.accountNo, "WITHDRAWAL", amount, r.balance).save();
        updateRecord(r);

        cout << "\n";
        printLine('*');
        cout << "  [✓] Rs. " << fixed << setprecision(2) << amount << " withdrawn successfully!\n"
             << "  Remaining Balance : Rs. " << r.balance << "\n";
        printLine('*');
        pauseScreen();
    }

    // ── 4. BALANCE CHECK ──
    void checkBalance() {
        printHeader("  BALANCE INQUIRY");

        AccountRecord r;
        if (!getVerifiedRecord(r)) return;

        cout << "\n";
        printLine();
        cout << "  Account No   : " << r.accountNo                            << "\n"
             << "  Holder Name  : " << r.holderName                           << "\n"
             << "  Account Type : " << r.accountType                          << "\n"
             << "  Balance      : Rs. " << fixed << setprecision(2) << r.balance << "\n";
        printLine();
        pauseScreen();
    }

    // ── 5. MINI STATEMENT (last 5 transactions) ──
    void miniStatement() {
        printHeader("  MINI STATEMENT");

        AccountRecord r;
        if (!getVerifiedRecord(r)) return;

        auto txns = Transaction::getByAccount(r.accountNo);
        if (txns.empty()) {
            cout << "\n  No transactions found for this account.\n";
            pauseScreen(); return;
        }

        // Show last 5
        int start = max(0, (int)txns.size() - 5);
        cout << "\n  Account : " << r.accountNo << "  |  " << r.holderName << "\n\n";
        printLine();
        cout << "  " << left
             << setw(20) << "Date & Time"
             << setw(14) << "Type"
             << setw(12) << "Amount"
             << "Balance\n";
        printLine();
        for (int i = start; i < (int)txns.size(); i++) {
            cout << "  " << left
                 << setw(20) << txns[i].timestamp
                 << setw(14) << txns[i].type
                 << "Rs." << setw(9) << fixed << setprecision(2) << txns[i].amount
                 << "Rs." << txns[i].balanceAfter << "\n";
        }
        printLine();
        cout << "  Showing last " << (txns.size() - start) << " of " << txns.size() << " transactions.\n";
        pauseScreen();
    }

    // ── 6. DISPLAY ALL ACCOUNTS (Admin view) ──
    void displayAll() {
        printHeader("  ALL ACCOUNTS  [ADMIN VIEW]");

        auto list = loadAll();
        if (list.empty()) {
            cout << "  No accounts found.\n";
            pauseScreen(); return;
        }

        printLine();
        cout << "  " << left
             << setw(13) << "Acc No"
             << setw(22) << "Name"
             << setw(12) << "Phone"
             << setw(10) << "Type"
             << "Balance\n";
        printLine();
        for (const auto& r : list) {
            cout << "  " << left
                 << setw(13) << r.accountNo
                 << setw(22) << r.holderName
                 << setw(12) << r.phone
                 << setw(10) << r.accountType
                 << "Rs." << fixed << setprecision(2) << r.balance << "\n";
        }
        printLine();
        cout << "  Total Accounts : " << list.size() << "\n";
        pauseScreen();
    }

    // ── 7. DELETE ACCOUNT ──
    void deleteAccount() {
        printHeader("  DELETE ACCOUNT");

        AccountRecord r;
        if (!getVerifiedRecord(r)) return;

        cout << "\n  Account found:\n";
        printLine();
        cout << "  Account No : " << r.accountNo  << "\n"
             << "  Name       : " << r.holderName  << "\n"
             << "  Balance    : Rs. " << fixed << setprecision(2) << r.balance << "\n";
        printLine();

        cout << "  Are you sure you want to DELETE this account? (y/n) : ";
        char c; cin >> c; cin.ignore();

        if (tolower(c) == 'y') {
            auto list = loadAll();
            list.erase(remove_if(list.begin(), list.end(),
                [&](const AccountRecord& x){ return x.accountNo == r.accountNo; }),
                list.end());
            saveAll(list);
            cout << "\n  [✓] Account deleted successfully.\n";
        } else {
            cout << "\n  Deletion cancelled.\n";
        }
        pauseScreen();
    }
};

// ─────────────────────────────────────────────
//  MAIN MENU
// ─────────────────────────────────────────────
void showMenu() {
    printHeader("  BANK MANAGEMENT APPLICATION");
    cout << "   1.  Create New Account\n"
         << "   2.  Deposit Money\n"
         << "   3.  Withdraw Money\n"
         << "   4.  Check Balance\n"
         << "   5.  Mini Statement\n"
         << "   6.  Display All Accounts\n"
         << "   7.  Delete Account\n"
         << "   8.  Exit\n\n";
    printLine();
    cout << "  Enter your choice (1-8) : ";
}

// ─────────────────────────────────────────────
//  ENTRY POINT
// ─────────────────────────────────────────────
int main() {
    Bank bank;
    int choice;

    while (true) {
        showMenu();

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
        }

        switch (choice) {
            case 1: bank.createAccount(); break;
            case 2: bank.deposit();       break;
            case 3: bank.withdraw();      break;
            case 4: bank.checkBalance();  break;
            case 5: bank.miniStatement(); break;
            case 6: bank.displayAll();    break;
            case 7: bank.deleteAccount(); break;
            case 8:
                clearScreen();
                printLine('=');
                cout << "  Thank you for using Bank Management Application!\n";
                printLine('=');
                return 0;
            default:
                cout << "\n  [!] Invalid choice. Enter a number between 1 and 8.\n";
                pauseScreen();
        }
    }
}
