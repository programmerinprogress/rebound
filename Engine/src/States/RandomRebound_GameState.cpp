#include "States/RandomRebound_GameState.h"
#include "Util/ImageResourceManager.h"
#include "Util/AudioResourceManager.h"
#include<ctime>
#include <sstream>

RandomRebound_GameState::RandomRebound_GameState():GameState(NULL , STATE_RANDOMREBOUND_GAMELEVEL)
{
}
RandomRebound_GameState::~RandomRebound_GameState()
{
    if(m_gameBall)
    {
        delete m_gameBall;
        m_gameBall = NULL;
    }
    if(m_player)
    {
        delete m_player;
        m_player = NULL;
    }
    if(m_opponent)
    {
        delete m_opponent;
        m_opponent = NULL;
    }
    if(m_backdrop)
    {
        delete m_backdrop;
        m_backdrop = NULL;
    }
    for(unsigned int i = 0; i < m_crates.size(); i++)
    {
        if(m_crates[i])
        {
            delete m_crates[i];
            m_crates[i] = NULL;
        }
    }
    if(m_playerScoreText)
    {
        delete m_playerScoreText;
        m_playerScoreText = NULL;
    }
    if(m_opponentScoreText)
    {
        delete m_opponentScoreText;
        m_opponentScoreText = NULL;
    }
}
void RandomRebound_GameState::CheckEvent()
{

    const sf::Input& input = m_engineInstance->GetGameWindow()->GetRawWindow()->GetInput();
    // grab X axis of 1st joystick (xbox left thumb stick) and check angle
    float joystick1X = input.GetJoystickAxis(0, sf::Joy::AxisX);

    if((input.IsKeyDown(sf::Key::Left) || (joystick1X < -85)) && !Paused())
    {
        RotatedRectangle playerBounds = m_player->Bounds();
        if(playerBounds.X()-m_player->XSpeed() > m_minPaddleX)
        {
            m_player->Move(DIRECTION_LEFT);
        }
        else if(playerBounds.X()-m_player->XSpeed() < m_player->XSpeed())
        {
            m_player->SetPosition(m_minPaddleX, playerBounds.Y());
        }
    }
    if((input.IsKeyDown(sf::Key::Right) || (joystick1X > 85)) && !Paused())
    {
        RotatedRectangle playerBounds = m_player->Bounds();

        if(playerBounds.X()+playerBounds.Width() < m_maxPaddleX)
        {
            m_player->Move(DIRECTION_RIGHT);
        }
        if(playerBounds.X()+playerBounds.Width() + m_player->XSpeed() >= m_maxPaddleX)
        {
            m_player->SetPosition(m_maxPaddleX - playerBounds.Width(), playerBounds.Y());
        }
    }

    while(m_engineInstance->GetGameWindow()->QueuedEvents())
    {
        // Get next event in event queue
        sf::Event* Event = m_engineInstance->GetGameWindow()->GetEvent();


        if(Event->Type == sf::Event::Closed)
        {
            this->m_engineInstance->Running(false);
            EndState();
            m_engineInstance->GetGameWindow()->Close();
        }
        else if(Event->Type == sf::Event::MouseButtonPressed)
        {
            switch(Event->MouseButton.Button)
            {
            case sf::Mouse::Left:
            {
                break;
            }
            default:
                break;
            }
        }
        else if(Event->Type == sf::Event::KeyPressed)
        {
            {
                switch(Event->Key.Code)
                {
                case sf::Key::P:
                {
                    Paused(!Paused());
                    break;
                }
                case sf::Key::Escape:
                {
                    this->m_engineInstance->Running(false);
                    EndState();
                    m_engineInstance->GetGameWindow()->Close();
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}
void RandomRebound_GameState::UpdateLogic()
{

    // make ball move at pre-defined speeds
    m_gameBall->Move(DIRECTION_AUTO);
    PerformMove();
    CheckOpponentBounds();
    // paddle collision

    if(m_gameBall->Collision(m_player->Bounds()) && !m_playerIsServer)
    {
        float newX, centreX;

        newX = 0;
        centreX = m_gameBall->Bounds().X()+ (m_gameBall->Bounds().Width()/2);
        newX = centreX - m_player->Bounds().X();
        newX *= REBOUND_SCALE_FACTOR;
        newX -= 10;
        m_gameBall->XSpeed(newX);
        m_gameBall->YSpeed(-m_gameBall->YSpeed());
        m_gameBall->SetPosition(m_gameBall->Bounds().X(), m_player->Bounds().Y()-1);

        if(m_gameBall->YSpeed() != -14 && m_gameBall->YSpeed() != 14)
        {
            (m_gameBall->YSpeed() > 0) ?  m_gameBall->YSpeed(m_gameBall->YSpeed() + 1) : m_gameBall->YSpeed(m_gameBall->YSpeed() -1);
        }
        m_playerIsServer = true;
        PlanMove();
        m_paddleBash.Play();

    }
    if(m_gameBall->Collision(m_opponent->Bounds()) && m_playerIsServer)
    {
        float newX, centreX;
        newX = 0;
        centreX = m_gameBall->Bounds().X()+ (m_gameBall->Bounds().Width()/2);
        newX = centreX - m_opponent->Bounds().X();
        newX *= REBOUND_SCALE_FACTOR;
        newX -= 10;
        m_gameBall->XSpeed(newX);
        m_gameBall->YSpeed(-m_gameBall->YSpeed());
       m_gameBall->SetPosition(m_gameBall->Bounds().X(), m_opponent->Bounds().Y() + m_opponent->Bounds().Height() + 1);

        if(m_gameBall->YSpeed() != -14 && m_gameBall->YSpeed() != 14)
        {
            (m_gameBall->YSpeed() > 0) ?  m_gameBall->YSpeed(m_gameBall->YSpeed() + 1) : m_gameBall->YSpeed(m_gameBall->YSpeed() -1);
        }
        m_playerIsServer = false;
        m_paddleBash.Play();

    }
    // horizontal bounds checks
    if((m_gameBall->Bounds().X()) < 16)
    {
        m_gameBall->SetPosition(17, m_gameBall->Bounds().Y());
        m_gameBall->XSpeed(-m_gameBall->XSpeed());
    }
     if((m_gameBall->Bounds().X()+m_gameBall->Bounds().Width()) > 624)
    {
        m_gameBall->SetPosition((624 - 17), m_gameBall->Bounds().Y());
        m_gameBall->XSpeed(-m_gameBall->XSpeed());
    }

    // vertical bounds checks
    if((m_gameBall->Bounds().Y()+m_gameBall->Bounds().Height()) > 464)
    {

        m_playerIsServer = true;
        m_gameBall->YSpeed(-BALL_BASE_SPEED);
        m_gameBall->SetPosition(320,240);

        std::stringstream ss;

        (!m_playerIsServer)? m_player->AddScore(1) : m_opponent->AddScore(1);
        PlanMove();
        m_goalCheer.Play();
        UpdateScoreBoards();

    }
    else if((m_gameBall->Bounds().Y()) <= 16)
    {
        m_gameBall->SetPosition(320,240);
        m_playerIsServer = false;
        m_gameBall->YSpeed(BALL_BASE_SPEED);
        (!m_playerIsServer)? m_player->AddScore(1) : m_opponent->AddScore(1);
        m_goalCheer.Play();
        UpdateScoreBoards();
    }

    for(unsigned int i = 0; i < m_crates.size(); i++)
    {
        Paddle* servingPlayer = NULL;
        Paddle* nonServingPlayer = NULL;

        if(m_gameBall->Collision(m_crates[i]->Bounds()) && m_crates[i]->Visible())
        {
            m_gameBall->YSpeed(-m_gameBall->YSpeed());
            m_gameBall->XSpeed(-m_gameBall->XSpeed());
            m_crates[i]->Visible(false);

            if(m_playerIsServer)
            {
                PlanMove();
            }
            if(m_playerIsServer)
            {
                servingPlayer = m_player;
                nonServingPlayer = m_opponent;
            }
            else
            {
                servingPlayer = m_opponent;
                nonServingPlayer = m_player;
            }

            // check for crate type, if random, regenerate
            if(m_crates[i]->Type() == CRATE_RANDOM)
            {
                int randomCrate;
                do
                {
                    randomCrate = 0 + (int)rand()/((int)RAND_MAX/(7-0));
                }
                while(randomCrate == 0);

                m_crates[i]->Type(randomCrate);
            }
            switch(m_crates[i]->Type())
            {
            case CRATE_CHANGETHEME:
            {
                int theme;
                do
                {
                    theme = 0 + (int)rand()/((int)RAND_MAX/(3-0));
                }
                while(theme == m_currentTheme);
                m_currentTheme = theme;
                ChangeTheme(m_currentTheme);
                break;
            }
            case CRATE_SPEEDUPBALL:
            {
                if(m_gameBall->YSpeed() != 16 &&  m_gameBall->YSpeed() != -16)
                {
                    (m_gameBall->YSpeed() > 0) ? m_gameBall->YSpeed(15) : m_gameBall->YSpeed(-15);
                }
                break;
            }
            case CRATE_SLOWBALL:
            {
                if(m_gameBall->YSpeed() != 1 &&  m_gameBall->YSpeed() != -1)
                {
                    (m_gameBall->YSpeed() > 0) ? m_gameBall->YSpeed(BALL_BASE_SPEED): m_gameBall->YSpeed(-BALL_BASE_SPEED);
                }
                break;
            }
            case CRATE_ONEUP:
            {
                servingPlayer->AddScore(1);
                UpdateScoreBoards();
                break;
            }
            case CRATE_ONEDOWN:
            {
                if(servingPlayer->Score() > 0)
                {
                    servingPlayer->AddScore(-1);
                    UpdateScoreBoards();
                }
                break;
            }
            case CRATE_TAKEPOINT:
            {
                if(nonServingPlayer->Score() > 0)
                {
                    servingPlayer->AddScore(1);
                    nonServingPlayer->AddScore(-1);
                    UpdateScoreBoards();
                }
                break;
            }
            case CRATE_REGENERATE:
            {
                RegenerateCrates();
                break;
            }
            default:
                break;
            }
            m_playerIsServer = !m_playerIsServer;
        }
    }

    // victory conditions for player 1 and 2
    if(m_player->Score() >= 6)
    {
        m_engineInstance->SetInt(STATE_GAMEOVER, "Outcome",1);
        EndState();
        m_engineInstance->SwitchState(STATE_GAMEOVER);
    }
    if(m_opponent->Score() >= 6)
    {
        m_engineInstance->SetInt(STATE_GAMEOVER, "Outcome",0);
        EndState();
        m_engineInstance->SwitchState(STATE_GAMEOVER);
    }
}
void RandomRebound_GameState::Paint()const
{
    GameWindow* win;
    win = m_engineInstance->GetGameWindow();

    win->Clear(sf::Color(128,128,255));
    win->Draw(m_backdrop);

    win->Draw(m_playerScoreText);
    win->Draw(m_opponentScoreText);

    m_gameBall->Draw(*win);
    m_player->Draw(*win);
    m_opponent->Draw(*win);
    for(int i = 0; i < 12; i++)
    {
        m_crates[i]->Draw(*win);
    }
}
void RandomRebound_GameState::InitState()
{
    // paddle bounds
    m_maxPaddleX = 624;
    m_minPaddleX = 16;

    srand ( time(NULL) );

    ImageResourceManager::LoadImageResource("ball_temple", "img/ball_temple.png");
    ImageResourceManager::LoadImageResource("ball_retro", "img/ball_retro.png");
    ImageResourceManager::LoadImageResource("ball_stadium", "img/ball_stadium.png");

    ImageResourceManager::LoadImageResource("paddle_temple", "img/paddle_temple.png");
    ImageResourceManager::LoadImageResource("paddle_retro", "img/paddle_retro.png");
    ImageResourceManager::LoadImageResource("paddle_stadium_red", "img/paddle_stadium_red.png");
    ImageResourceManager::LoadImageResource("paddle_stadium_blue", "img/paddle_stadium_blue.png");

    ImageResourceManager::LoadImageResource("numbers_temple", "img/Numbersheet_Temple.png");
    ImageResourceManager::LoadImageResource("numbers_retro", "img/Numbersheet_Retro.png");
    ImageResourceManager::LoadImageResource("numbers_stadium_red", "img/Numbersheet_Stadium_Red.png");
    ImageResourceManager::LoadImageResource("numbers_stadium_blue", "img/Numbersheet_Stadium_Blue.png");

    ImageResourceManager::LoadImageResource("crate_temple","img/crate_temple.png");
    ImageResourceManager::LoadImageResource("crate_retro","img/crate_retro.png");
    ImageResourceManager::LoadImageResource("crate_stadium","img/crate_stadium.png");
    // themes
    ImageResourceManager::LoadImageResource("backdrop_temple","img/backdrop_temple.png");
    ImageResourceManager::LoadImageResource("backdrop_retro","img/backdrop_retro.png");
    ImageResourceManager::LoadImageResource("backdrop_stadium","img/backdrop_stadium.png");

    // sounds
    AudioResourceManager::LoadSoundResource("blip_retro", "sounds/blip_retro.wav");
    AudioResourceManager::LoadSoundResource("blip_stadium", "sounds/blip_stadium.wav");
    AudioResourceManager::LoadSoundResource("blip_temple", "sounds/blip_temple.wav");

    AudioResourceManager::LoadSoundResource("goal_temple", "sounds/goal_temple.wav");
    AudioResourceManager::LoadSoundResource("goal_retro", "sounds/goal_retro.wav");
    AudioResourceManager::LoadSoundResource("goal_stadium", "sounds/goal_stadium.wav");

    m_gameBall = new Ball(Rect(312,232,16,16), Point(0,0), ImageResourceManager::GetImageResource("ball_retro"));
    m_player = new Paddle(Rect(280,438,96,16), Point(0,0), ImageResourceManager::GetImageResource("paddle_retro"));
    m_opponent = new Paddle(Rect(280,32,96,16), Point(0,0), ImageResourceManager::GetImageResource("paddle_retro"));
    m_backdrop = new GameSprite(Rect(0,0,640,480), Point(0,0), ImageResourceManager::GetImageResource("backdrop_retro"));

    m_playerScoreText = new GameSprite(Rect(80,120,160,240),Point(0,0), ImageResourceManager::GetImageResource(("numbers_temple")));
    m_opponentScoreText = new GameSprite(Rect(400,120,160,240),Point(0,0), ImageResourceManager::GetImageResource(("numbers_temple")));
    /* fill  play area with random crates dotted around */
    int crateMinX, crateMinY, crateWidth, crateHeight, crateType;
    crateMinX = 16;
    crateMinY = 64;
    crateWidth = 32;
    crateHeight = 32;

    int selectedCrateX, selectedCrateY;
    bool unique;
    int crateCount(0);

    vector<Point*> selectedPoints;
    // finding an excuse to do...while since 2009 ;)
    // create Point pairs for placing crates, avoid duplicates
    do
    {
        selectedCrateX = 0 + (int)rand()/((int)RAND_MAX/(19-0));
        selectedCrateY = 0 + (int)rand()/((int)RAND_MAX/(11-0));
        unique = true;

        for(unsigned int j = 0; j < selectedPoints.size(); j++)
        {
            if(selectedPoints[j]->X() == selectedCrateX && selectedPoints[j]->Y() == selectedCrateY)
            {
                unique = false;
                break;
            }
        }
        if(unique)
        {
            crateCount++;
            selectedPoints.push_back(new Point(selectedCrateX,selectedCrateY));
        }
    }
    while(crateCount != 12);

    // add crates to play area
    for(unsigned int i = 0; i < selectedPoints.size(); i++)
    {
        crateType = 0 + (int)rand()/((int)RAND_MAX/(8-0));
        m_crates.push_back(new Crate(Rect((selectedPoints[i]->X() * crateWidth)+crateMinX,(selectedPoints[i]->Y() * crateHeight)+crateMinY,crateWidth,crateHeight),
                                     crateType, ImageResourceManager::GetImageResource("crate_retro")));
    }
    m_playerScoreText->CurrentFrame( m_opponent->Score());
    m_opponentScoreText->CurrentFrame( m_opponent->Score());

    //select random theme
    m_currentTheme = 0 + (int)rand()/((int)RAND_MAX/(3-0));
    ChangeTheme(m_currentTheme);
}
void RandomRebound_GameState::EndState()
{
}
void RandomRebound_GameState::StartState()
{
    m_playerIsServer = false;
    m_player->Score(0);
    m_opponent->Score(0);
    m_player->SetPosition(280,438);
    m_opponent->SetPosition(280,32);
    m_gameBall->SetPosition(320,240);

    m_gameBall->YSpeed(BALL_BASE_SPEED);
    m_gameBall->XSpeed(0);
    RegenerateCrates();
    UpdateScoreBoards();
}
void RandomRebound_GameState::ExitState()
{
}
void RandomRebound_GameState::PauseState()
{
}
void RandomRebound_GameState::ResumeState()
{
}
void RandomRebound_GameState::SetInt(const std::string& key, int value)
{
}
void RandomRebound_GameState::SetFloat(const std::string& key, float value)
{
}
void RandomRebound_GameState::SetString(const std::string& key, const std::string& value)
{
}
void RandomRebound_GameState::SetObject(const std::string& key, void* value)
{
}
void RandomRebound_GameState::PlanMove()
{
    //very very simple pong AI, enemy paddle randomly selects part of paddle to hit ball with
    float low, high;
    float r;
    low = 5;
    high = 91;
    r = low + (float)rand()/((float)RAND_MAX/(high-low));
    m_opponentsTarget = r;
}
void RandomRebound_GameState::PerformMove()
{
    // paddle waits until it has a chance of serving (emulates a very bad human player)
    if(m_playerIsServer)
    {
        if((m_opponent->Bounds().X() + (m_opponentsTarget) < (m_gameBall->Bounds().X() + (m_gameBall->Bounds().Width()/2))))
        {
            m_opponent->Move(DIRECTION_RIGHT);
        }
        if((m_opponent->Bounds().X()+ (m_opponentsTarget) > (m_gameBall->Bounds().X() + (m_gameBall->Bounds().Width()/2))))
        {
            m_opponent->Move(DIRECTION_LEFT);
        }
    }

}
void RandomRebound_GameState::UpdateScoreBoards()
{
    m_opponentScoreText->CurrentFrame(m_opponent->Score());
    m_playerScoreText->CurrentFrame(m_player->Score());
}
void RandomRebound_GameState::CheckOpponentBounds()
{
    RotatedRectangle playerBounds = m_opponent->Bounds();
    if(playerBounds.X()-m_opponent->XSpeed() > m_minPaddleX)
    {
        m_opponent->Move(DIRECTION_LEFT);
    }
    else if(playerBounds.X()-m_opponent->XSpeed() < m_opponent->XSpeed())
    {
        m_opponent->SetPosition(m_minPaddleX, playerBounds.Y());
    }
    if(playerBounds.X()+playerBounds.Width() < m_maxPaddleX)
    {
        m_opponent->Move(DIRECTION_RIGHT);
    }
    else if(playerBounds.X()+playerBounds.Width() + m_opponent->XSpeed() >= m_maxPaddleX)
    {
        m_opponent->SetPosition(m_maxPaddleX - playerBounds.Width(), playerBounds.Y());
    }
}
void RandomRebound_GameState::ChangeTheme(int theme)
{
    switch(theme)
    {
    case THEME_TEMPLE:
    {
        for(unsigned int i = 0; i < m_crates.size(); i++)
        {
            m_crates[i]->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("crate_temple"));
        }
        m_backdrop->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("backdrop_temple"));
        m_player->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("paddle_temple"));
        m_opponent->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("paddle_temple"));
        m_gameBall->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("ball_temple"));
        m_paddleBash.SetBuffer(AudioResourceManager::GetSoundResource("blip_temple"));
        m_goalCheer.SetBuffer(AudioResourceManager::GetSoundResource("goal_temple"));

        m_playerScoreText->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("numbers_temple"));
        m_opponentScoreText->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("numbers_temple"));

        break;
    }
    case THEME_RETRO:
    {
        for(unsigned int i = 0; i < m_crates.size(); i++)
        {
            m_crates[i]->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("crate_retro"));
        }
        m_backdrop->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("backdrop_retro"));
        m_player->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("paddle_retro"));
        m_opponent->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("paddle_retro"));
        m_gameBall->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("ball_retro"));

        m_paddleBash.SetBuffer(AudioResourceManager::GetSoundResource("blip_retro"));
        m_goalCheer.SetBuffer(AudioResourceManager::GetSoundResource("goal_retro"));

        m_playerScoreText->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("numbers_retro"));
        m_opponentScoreText->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("numbers_retro"));

        break;
    }
    case THEME_STADIUM:
    {
        for(unsigned int i = 0; i < m_crates.size(); i++)
        {
            m_crates[i]->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("crate_stadium"));

        }
        m_backdrop->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("backdrop_stadium"));
        m_player->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("paddle_stadium_blue"));
        m_opponent->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("paddle_stadium_red"));
        m_gameBall->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("ball_stadium"));

        m_paddleBash.SetBuffer(AudioResourceManager::GetSoundResource("blip_stadium"));
        m_goalCheer.SetBuffer(AudioResourceManager::GetSoundResource("goal_stadium"));


        m_playerScoreText->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("numbers_stadium_blue"));
        m_opponentScoreText->GetBaseSprite()->SetImage(ImageResourceManager::GetImageResource("numbers_stadium_red"));
        break;
    }
    default:
        break;
    }
}
void RandomRebound_GameState::RegenerateCrates()
{
    /* fill  play area with random crates dotted around */
    int crateMinX, crateMinY, crateWidth, crateHeight, crateType;
    crateMinX = 16;
    crateMinY = 64;
    crateWidth = 32;
    crateHeight = 32;

    int selectedCrateX, selectedCrateY;
    bool unique;
    int crateCount(0);

    vector<Point*> selectedPoints;
    // finding an excuse to do...while since 2009 ;)
    // create Point pairs for placing crates, avoid duplicates
    do
    {
        selectedCrateX = 0 + (int)rand()/((int)RAND_MAX/(18-0));
        selectedCrateY = 0 + (int)rand()/((int)RAND_MAX/(10-0));
        unique = true;

        for(unsigned int j = 0; j < selectedPoints.size(); j++)
        {
            if(selectedPoints[j]->X() == selectedCrateX && selectedPoints[j]->Y() == selectedCrateY)
            {
                unique = false;
                break;
            }
        }
        if(unique)
        {
            crateCount++;
            selectedPoints.push_back(new Point(selectedCrateX,selectedCrateY));
        }
    }
    while(crateCount != 12);

    // add crates to play area
    for(unsigned int i = 0; i < m_crates.size(); i++)
    {
        crateType = 0 + (int)rand()/((int)RAND_MAX/(8-0));
        m_crates[i]->Visible(true);
        m_crates[i]->SetPosition((selectedPoints[i]->X() * crateWidth)+crateMinX,(selectedPoints[i]->Y() * crateHeight)+crateMinY);
    }
}

