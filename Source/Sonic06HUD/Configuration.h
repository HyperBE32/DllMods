#pragma once

class Configuration
{
public:
    static bool load(const std::string& rootPath);

    static bool m_usingSTH2006Project;
    static int m_characterIcon;
    static bool m_uiColor;
};
