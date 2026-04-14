#include "ChronoGraph.h"
#include <cctype>
#include <stdexcept>



struct Parser {
    const std::wstring& src;
    size_t pos;
    double vx, vy, vz, vt;

    Parser(const std::wstring& s, double x, double y, double z, double t)
        : src(s), pos(0), vx(x), vy(y), vz(z), vt(t) {}

    wchar_t peek() {
        while (pos < src.size() && src[pos] == L' ') pos++;
        return pos < src.size() ? src[pos] : 0;
    }
    wchar_t get() {
        while (pos < src.size() && src[pos] == L' ') pos++;
        return pos < src.size() ? src[pos++] : 0;
    }
    bool match(wchar_t c) {
        if (peek() == c) { get(); return true; }
        return false;
    }

    double parseExpr();   // +/-
    double parseTerm();   // * /
    double parsePower();  // ^
    double parseUnary();  // signo unario
    double parsePrimary();// numero | ( expr ) | funcion | variable

    std::wstring parseIdent() {
        std::wstring id;
        while (pos < src.size() && (iswalpha(src[pos]) || src[pos] == L'_' || (id.size() && iswdigit(src[pos]))))
            id += src[pos++];
        // convertir a minusculas
        for (auto& c : id) c = towlower(c);
        return id;
    }
    double parseNumber() {
        std::wstring num;
        while (pos < src.size() && (iswdigit(src[pos]) || src[pos] == L'.'))
            num += src[pos++];
        if (num.empty()) throw std::runtime_error("expected number");
        return _wtof(num.c_str());
    }
};

double Parser::parseExpr() {
    double v = parseTerm();
    for (;;) {
        if (match(L'+'))      v += parseTerm();
        else if (match(L'-')) v -= parseTerm();
        else break;
    }
    return v;
}
double Parser::parseTerm() {
    double v = parsePower();
    for (;;) {
        if (match(L'*'))      v *= parsePower();
        else if (match(L'/')) { double d = parsePower(); v = (d == 0) ? 1e18 : v / d; }
        else break;
    }
    return v;
}
double Parser::parsePower() {
    double base = parseUnary();
    if (match(L'^')) {
        double exp = parseUnary(); // asociativo a la derecha mediante recursion para ^
        return std::pow(base, exp);
    }
    return base;
}
double Parser::parseUnary() {
    if (match(L'-')) return -parsePrimary();
    if (match(L'+')) return parsePrimary();
    return parsePrimary();
}
double Parser::parsePrimary() {
    wchar_t c = peek();
    if (c == L'(') { get(); double v = parseExpr(); match(L')'); return v; }
    if (iswdigit(c) || c == L'.') return parseNumber();
    if (iswalpha(c) || c == L'_') {
        std::wstring id = parseIdent();
        // variables
        if (id == L"x")  return vx;
        if (id == L"y")  return vy;
        if (id == L"z")  return vz;
        if (id == L"t")  return vt;
        if (id == L"pi") return M_PI;
        if (id == L"e")  return M_E;
        // funciones
        if (peek() == L'(') {
            match(L'(');
            double a = parseExpr();
            double b = 0;
            if (peek() == L',') { match(L','); b = parseExpr(); }
            match(L')');
            if (id == L"sin")   return std::sin(a);
            if (id == L"cos")   return std::cos(a);
            if (id == L"tan")   return std::tan(a);
            if (id == L"asin")  return std::asin(a);
            if (id == L"acos")  return std::acos(a);
            if (id == L"atan")  return std::atan(a);
            if (id == L"atan2") return std::atan2(a, b);
            if (id == L"sqrt")  return (a < 0) ? 0 : std::sqrt(a);
            if (id == L"abs")   return std::abs(a);
            if (id == L"log" || id == L"ln") return std::log(std::abs(a));
            if (id == L"log2")  return std::log2(std::abs(a));
            if (id == L"log10") return std::log10(std::abs(a));
            if (id == L"exp")   return std::exp(a);
            if (id == L"floor") return std::floor(a);
            if (id == L"ceil")  return std::ceil(a);
            if (id == L"round") return std::round(a);
            if (id == L"sinh")  return std::sinh(a);
            if (id == L"cosh")  return std::cosh(a);
            if (id == L"tanh")  return std::tanh(a);
            if (id == L"sign")  return (a > 0) ? 1.0 : (a < 0) ? -1.0 : 0.0;
            if (id == L"min")   return std::min(a, b);
            if (id == L"max")   return std::max(a, b);
            if (id == L"mod")   return std::fmod(a, b);
            if (id == L"pow")   return std::pow(a, b);
            // funcion desconocida -> 0
            return 0.0;
        }
        // nombre suelto sin parentesis
        return 0.0;
    }
    return 0.0;
}

double EvaluateExpression(const std::wstring& expr, double x, double y, double z, double t) {
    try {
        // El alias de mayusculas/minusculas para T se maneja en parseIdent
        Parser p(expr, x, y, z, t);
        return p.parseExpr();
    } catch (...) {
        return 0.0;
    }
}
