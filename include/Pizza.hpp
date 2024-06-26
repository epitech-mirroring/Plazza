/*
** EPITECH PROJECT, 2024
** Plazza
** File description:
** No file there , just an epitech header example .
** You can even have multiple lines if you want !
*/

#pragma once
#include <iostream>

class Pizza {
public:
    enum Type {
        Regina = 1,
        Margarita = 2,
        Americana = 4,
        Fantasia = 8
    };

    enum Size {
        S = 1,
        M = 2,
        L = 4,
        XL = 8,
        XXL = 16
    };

protected:
    Type _type;
    Size _size;

public:
    Pizza(Type type, Size size);
    ~Pizza();

    Type getType() const;
    Size getSize() const;
    void setType(Type type);
    void setSize(Size size);

    bool operator==(const Pizza &other) const;
    bool operator!=(const Pizza &other) const;

    friend std::ostream &operator<<(std::ostream &os, const Pizza &pizza);
    friend std::ostream &operator<<(std::ostream &os, const Type &type);
    friend std::ostream &operator<<(std::ostream &os, const Size &size);
};
