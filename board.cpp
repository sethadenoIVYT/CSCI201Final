/*

Author: Seth Deno
Date Written: 12/13/2024
Assignment: Final Project
Short Desc: This program displays a gui to the user, including a home, how to play
game, and results screen. The game is a 5x5 grid of buttons in which the user has to
find all of the five "safe" buttons within the grid consecutively. The user has a 
set number of chances to do this, the more chances it takes them, the less points 
they will earn. A result page will pop up when the game is over, telling you if you 
won or lost and what your score was if you won. The safe buttons are shown by the 
middle of the button turning green when you press it.

*/

#include <gtkmm.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <array>

using namespace std;

int chancesLeft = 12;
int score = 0;

bool safeClicked[5] = {false, false, false, false, false};

class ResultWindow : public Gtk::Window {
public:
    ResultWindow() {
        set_title("Result");
        set_default_size(300, 300);

        string showHigh;
        int highScore;

        Gtk::Box* resultBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

        if (chancesLeft == 0) {
            gameResult.set_text("Better Luck Next Time.");
        } else {
            gameResult.set_text("Congrats! Your Final Score: " + to_string(score));

            ifstream ReadFile("highScore.txt");
            getline(ReadFile, showHigh);

            highScore = stoi(showHigh);

            if (highScore > score) {
                highestScore.set_text("The All Time High Score Is: " + showHigh);
            } else {
                highestScore.set_text("The All Time High Score Is: " + to_string(score));
                ofstream replaceFile("highScore.txt");
                replaceFile << to_string(score);
            }
        }

        gameResult.set_margin_bottom(20);
        gameResult.set_margin_top(20);
        gameResult.set_halign(Gtk::ALIGN_CENTER);

        highestScore.set_margin_bottom(20);
        highestScore.set_margin_top(20);
        highestScore.set_halign(Gtk::ALIGN_CENTER);

        resultBox->pack_start(gameResult, Gtk::PACK_SHRINK);
        resultBox->pack_start(highestScore, Gtk::PACK_SHRINK);

        add(*resultBox);
        show_all_children();
    }

    virtual ~ResultWindow() {}

protected:
    Gtk::Label gameResult;
    Gtk::Label highestScore;
};

class HowToPlay : public Gtk::Window {
public:
    HowToPlay() {
        set_title("How To Play");
        set_default_size(400, 400);

        Gtk::Label* gameInfo = Gtk::manage(new Gtk::Label(
            "GAMEPLAY: "
            "\n\n"
            "The goal of this game is to click all the safe buttons consecutively before running out of "
            "chances. There are five safe buttons within the 5x5 grid of buttons, and one safe button per "
            "row of buttons. The button will let you know when a button is safe by changing the center of "
            "the button from black to green. Once you press a button that is not a safe button, all safe "
            "buttons that have been previously clicked, will revert to their non-clicked state. This means "
            "that the player should focus on memorizing the pattern of these safe buttons. "
            "\n\n"
            "SCORE:"
            "\n\n"
            "Your score is based on if you click a safe button. Clicking a safe button will reward you "
            "points and give you more points if you have more chances, the less chances you have left, "
            "the less points you will be rewarded. You will also lose a number of points for every non- "
            "safe button that you click."
            "\n\n"
            "END OF GAME:"
            "\n\n"
            "If you run out of chances, a screen will pop up telling you Better Luck Next Time. If you "
            "click all five safe buttons consecutively without running out of chances, a screen will pop "
            "up congratulating you on your victory. It will also tell you your score and list the all-time high "
            "score."
            "\n\n"
            "RESTART:"
            "\n\n"
            "To restart the game, simply press the START button located on the home page."
            "\n\n"
        ));

        gameInfo->set_line_wrap(true);
        gameInfo->set_max_width_chars(80);

        gameInfo->set_margin_top(10);
        gameInfo->set_margin_bottom(10);

        Gtk::ScrolledWindow* scrollableWindow = Gtk::manage(new Gtk::ScrolledWindow());
        scrollableWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scrollableWindow->add(*gameInfo);

        add(*scrollableWindow);
        show_all_children();
    }

    virtual ~HowToPlay() {}

protected:
    Gtk::Label gameInfo;
};

class gameWindow : public Gtk::Window {
public:
    gameWindow() {
        set_title("Safe Button Grid");
        set_default_size(500, 500);

        chancesLeft = 12;
        score = 0;

        Gtk::Box* gameBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
        add(*gameBox);

        gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
        gameInfo.set_margin_bottom(10);
        gameInfo.set_halign(Gtk::ALIGN_CENTER);

        gameBox->pack_start(gameInfo, Gtk::PACK_SHRINK);

        grid.set_row_homogeneous(true);
        grid.set_column_homogeneous(true);

        srand(time(0));

        for (int row = 0; row < 5; ++row) {
            int safe = (rand() % 5) + 1;
            safe_buttons.push_back(safe);
        }

        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 5; ++col) {
                Gtk::Button* button = new Gtk::Button("*");

                button->set_hexpand(true);
                button->set_vexpand(true);

                button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &gameWindow::on_button_clicked), button, row, col));

                grid.attach(*button, col, row, 1, 1);
                buttons.push_back(button);
            }
        }
        gameBox->pack_start(grid, Gtk::PACK_EXPAND_WIDGET);
        apply_css();
        show_all_children();
    }

    virtual ~gameWindow() {}

protected:
    Gtk::Grid grid;
    Gtk::Label gameInfo;
    vector<int> safe_buttons;
    vector<Gtk::Button*> buttons;

    void apply_css() {
        Glib::RefPtr<Gtk::CssProvider> provider = Gtk::CssProvider::create();
        provider->load_from_data(
            "button.safe { background-color: green; color: green; }"
            "button.notSafe { background-color: red; color: red; }"
            "button { font-size: 48px; }"
        );

        Gtk::StyleContext::add_provider_for_screen(
            Gdk::Screen::get_default(),
            provider,
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );
    }

    void on_button_clicked(Gtk::Button* button, int row, int col) {
        if (safe_buttons[row] == col + 1) {
            if (chancesLeft == 12) {
                score = score + 250;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 11) {
                score = score + 200;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 10) {
                score = score + 150;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 9) {
                score = score + 90;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 8) {
                score = score + 80;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 7) {
                score = score + 70;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 6) {
                score = score + 60;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 5) {
                score = score + 50;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 4) {
                score = score + 40;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 3) {
                score = score + 30;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 2) {
                score = score + 20;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            } else if (chancesLeft == 1) {
                score = score + 10;
                gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
            }  
            button->get_style_context()->add_class("safe");

            safeClicked[row] = true;

            if ((safeClicked[0] == true) && (safeClicked[1] == true) && (safeClicked[2] == true) && (safeClicked[3] == true) && (safeClicked[4] == true)) {
                ResultWindow* mywindow = new ResultWindow();
                mywindow->show_all();
                close();
            }
        } else {
            chancesLeft = chancesLeft - 1;
            wrong_choice();
            if (chancesLeft == 0) {
                ResultWindow* mywindow = new ResultWindow();
                mywindow->show_all();
                close();
            }
        }

        button->set_sensitive(false);
    }

    void wrong_choice() {
        for (auto& button : buttons) {
            button->get_style_context()->remove_class("safe");
            button->get_style_context()->remove_class("notSafe");
            button->set_sensitive(true);
            button->set_label("*");
        }

        for (int change = 0; change < sizeof(safeClicked); change++) {
            safeClicked[change] = false;
        }
        gameInfo.set_text("Score: " + to_string(score) + "                 Chances Left: " + to_string(chancesLeft));
    }
};

class HomeView : public Gtk::Window { 
public:
    HomeView() {
        set_title("Home");
        set_default_size(200, 200);

        start_button.set_label("START");
        start_button.signal_clicked().connect(sigc::mem_fun(*this, &HomeView::on_start_button_clicked));

        how_to_button.set_label("HOW TO PLAY");
        how_to_button.signal_clicked().connect(sigc::mem_fun(*this, &HomeView::on_how_to_button_clicked));

        Gtk::Box* homeLayout = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
        homeLayout->pack_start(start_button, Gtk::PACK_SHRINK);
        homeLayout->pack_start(how_to_button, Gtk::PACK_SHRINK);

        add(*homeLayout);

        homeLayout->set_margin_top(40);
        homeLayout->set_margin_bottom(40);
        homeLayout->set_halign(Gtk::ALIGN_CENTER);
        show_all_children();
    }

    virtual ~HomeView() {}

protected:
    Gtk::Button start_button;
    Gtk::Button how_to_button;

    void on_start_button_clicked() {
        gameWindow* mywindow = new gameWindow();
        mywindow->show_all();
    }

    void on_how_to_button_clicked() {
        HowToPlay* howToPlayWindow = new HowToPlay();
        howToPlayWindow->show_all();
    }
};

int main(int argc, char* argv[]) {
    Gtk::Main app(argc, argv);

    HomeView startWindow;

    Gtk::Main::run(startWindow);

    return 0;
}
