# Bank Management Application in C++

## Overview
The Bank Management Application is a console-based banking system developed using C++ with Object-Oriented Programming and File Handling concepts. The application simulates core banking operations such as account creation, deposits, withdrawals, balance inquiry, and transaction management.

## Features
- Create Savings and Current Accounts
- Deposit Money
- Withdraw Money
- Balance Inquiry
- Mini Statement Generation
- Display All Accounts
- Delete Account
- PIN Verification System
- Transaction History
- Persistent Data Storage using File Handling

## Technologies Used
- C++
- Object-Oriented Programming (OOP)
- File Handling
- Binary File Storage

## File Storage
The application stores data persistently using:
- `accounts.dat` → Stores account details
- `transactions.dat` → Stores transaction history

## Account Types
### Savings Account
- Minimum balance: Rs.500
- Interest Rate: 4% per annum

### Current Account
- Overdraft limit: Rs.10,000

## Key Concepts Implemented
- Classes and Objects
- Inheritance
- Polymorphism
- Encapsulation
- File Handling
- Menu-Driven Programming

## How to Run
1. Compile the code:
```bash
g++ main.cpp -o bank
