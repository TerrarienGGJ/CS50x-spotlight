import os

from cs50 import SQL
from flask import Flask, flash, redirect, render_template, request, session
from flask_session import Session
from tempfile import mkdtemp
from werkzeug.security import check_password_hash, generate_password_hash

from helpers import apology, login_required, lookup, usd

# Configure application
app = Flask(__name__)

# Custom filter
app.jinja_env.filters["usd"] = usd

# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///finance.db")

# Make sure API key is set
if not os.environ.get("API_KEY"):
    raise RuntimeError("API_KEY not set")


@app.after_request
def after_request(response):
    """Ensure responses aren't cached"""
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


@app.route("/")
@login_required
def index():
    """Show portfolio of stocks"""
    id = session["user_id"]
    portfolio = db.execute("SELECT symbol, quantity FROM portfolios WHERE id = ? ORDER BY quantity", id)
    cash = db.execute("SELECT cash FROM users WHERE id = ?", id)[0]["cash"]
    total = cash
    for row in portfolio:
        total += lookup(row["symbol"])["price"] * row["quantity"]
    return render_template("index.html", portfolio=portfolio, lookup=lookup, cash=cash, total=total)


@app.route("/buy", methods=["GET", "POST"])
@login_required
def buy():
    """Buy shares of stock"""
    if request.method == "GET":
        return render_template("buy.html")
    else:
        quantity = request.form.get("shares")
        if not quantity.isdigit():
            return apology("Please fill a positive integer amount of shares you want to buy.")
        quantity = int(quantity)
        symbol = request.form.get("symbol").upper()
        symbol_quote = lookup(symbol)
        if symbol_quote == None:
            return apology("The stock symbol you tried to buy is not referenced")
        else:
            id = session.get("user_id")
            user = db.execute("SELECT * FROM users WHERE id = ?", id)
            if (quantity * symbol_quote["price"]) > user[0]["cash"]:
                return apology("You don't have the funds for you order.")
            else:
                cash = user[0]["cash"]
                db.execute("UPDATE users SET cash = ? WHERE id = ?", cash - (quantity * symbol_quote["price"]), id)
                db.execute("INSERT INTO history (username, type, symbol, quantity, price, total) VALUES (?, ?, ?, ?, ?, ?)", user[0]["username"], "BUY", symbol, quantity, symbol_quote["price"], quantity * symbol_quote["price"])
                stock_owned = db.execute("SELECT * FROM portfolios WHERE symbol = ? AND id = ?", symbol, id)
                if len(stock_owned) != 0:
                    db.execute("UPDATE portfolios SET quantity = ?", stock_owned[0]["quantity"] + quantity)
                else:
                    db.execute("INSERT INTO portfolios (id, username, symbol, quantity) VALUES (?, ?, ?, ?)", id, user[0]["username"], symbol, quantity)
    return redirect("/")


@app.route("/history")
@login_required
def history():
    """Show history of transactions"""
    id = session["user_id"]
    username = db.execute("SELECT username FROM users WHERE id = ?", id)
    history = db.execute("SELECT * FROM history WHERE username = ?", username[0]["username"])
    return render_template("history.html", history=history)


@app.route("/login", methods=["GET", "POST"])
def login():
    """Log user in"""
    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":

        # Ensure username was submitted
        if not request.form.get("username"):
            return apology("must provide username", 403)

        # Ensure password was submitted
        elif not request.form.get("password"):
            return apology("must provide password", 403)

        # Query database for username
        rows = db.execute("SELECT * FROM users WHERE username = ?", request.form.get("username"))

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(rows[0]["hash"], request.form.get("password")):
            return apology("invalid username and/or password", 403)

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    else:
        return render_template("login.html")


@app.route("/logout")
def logout():
    """Log user out"""

    # Forget any user_id
    session.clear()

    # Redirect user to login form
    return redirect("/")


@app.route("/quote", methods=["GET", "POST"])
@login_required
def quote():
    """Get stock quote."""
    if request.method == "GET":
        return render_template("quote.html")
    else:
        symbol = request.form.get("symbol").upper()
        symbol_quote = lookup(symbol)
        if symbol_quote is not None:
            return render_template("quoted.html", name=symbol_quote["name"], price=symbol_quote["price"], symbol=symbol_quote["symbol"])
    return apology("Couldn't find symbol's quote")


@app.route("/register", methods=["GET", "POST"])
def register():
    """Register user"""
    if request.method == "POST":
        user = ""
        if len(db.execute("SELECT username FROM users WHERE username = ?", request.form.get("username"))) == 0 :
            user = request.form.get("username")
        psw = ""
        psw = request.form.get("password")
        if len(user) == 0:
            return apology("Username already used or blank.")
        if len(psw) == 0 or psw != request.form.get("confirmation"):
            return apology("Password doesn't match the confirmation or blank.")
        db.execute("INSERT INTO users (username, hash) VALUES (?, ?)", user, generate_password_hash(psw))
        return redirect("/")
    else:
        return render_template("register.html")


@app.route("/sell", methods=["GET", "POST"])
@login_required
def sell():
    """Sell shares of stock"""
    id = session.get("user_id")
    if request.method == "GET":
        symbols = db.execute("SELECT symbol FROM portfolios WHERE id = ?", id)
        return render_template("sell.html", symbols=symbols)
    else:
        quantity = int(request.form.get("shares"))
        if quantity < 0:
            return apology("Please fill a positive amount of shares you want to sell.")
        symbol = request.form.get("symbol").upper()
        symbol_quote = lookup(symbol)
        available = db.execute("SELECT quantity FROM portfolios WHERE id = ? AND symbol = ?", id, symbol)
        if quantity > available[0]["quantity"]:
            return apology("You don't have enough shares to sell the selected amount.")
        else:
            user = db.execute("SELECT * FROM users WHERE id = ?", id)
            cash = user[0]["cash"]
            db.execute("UPDATE users SET cash = ? WHERE id = ?", cash + (quantity * symbol_quote["price"]), id)
            db.execute("INSERT INTO history (username, type, symbol, quantity, price, total) VALUES (?, ?, ?, ?, ?, ?)", user[0]["username"], "SELL", symbol, quantity, symbol_quote["price"], quantity * symbol_quote["price"])
            db.execute("UPDATE portfolios SET quantity = ? WHERE id = ? AND symbol = ?", available[0]["quantity"] - quantity, id, symbol)
        return redirect("/")

@app.route("/add", methods=["GET", "POST"])
@login_required
def add():
    if request.method == "GET":
        return render_template("add.html")
    else:
        cash_add = int(request.form.get("cash"))
        id = session.get("user_id")
        user = db.execute("SELECT * FROM users WHERE id = ?", id)
        db.execute("UPDATE users SET cash = ?", user[0]["cash"] + cash_add)
        return redirect("/")
    """Add cash to balance"""