#pragma once

template <int N>
class FixedString {
private:
    const int m_capacity;
    int m_final_index;
    int m_size;
    bool m_is_truncated;
    char m_string[N];

    int createString(const char* content) {
        if (content == nullptr) {
            m_string[0] = '\0';
            return 0;
        }

        int i = 0;
        
        for (; i < m_capacity - 1 && *content != '\0'; i++, content++) {
            m_string[i] = *content;
            m_size++;
        }

        m_string[i] = '\0';
        m_final_index = i;

        bool truncated = (i == m_capacity - 1) && (*content != '\0');
        if (truncated) m_is_truncated = true;

        return truncated ? -1 : 0;
    };

public:
    FixedString(const char* string_content) : m_capacity(N), m_final_index(0), m_size(0), m_is_truncated(false) {
        if (m_capacity <= 0) return;
        if (string_content == nullptr) { m_string[0] = '\0'; return; }
        if (createString(string_content) == -1) return;
    }

    int size() const { return m_size; }
    int capacity() const { return m_capacity-1; }
    bool is_truncated() const { return m_is_truncated; }

    int append(const char* content) {
        if (content == nullptr) return -1;
        if (m_final_index >= m_capacity - 1) return -1;

        int i = m_final_index;

        for (; i < m_capacity - 1 && *content != '\0'; i++, content++) {
            m_string[i] = *content;
            m_size++;
        }

        m_string[i] = '\0';
        m_final_index = i;

        bool truncated = (i == m_capacity - 1) && (*content != '\0');
        if (truncated) m_is_truncated = true;

        return truncated ? -1 : 0;
    }

    int replace(int start_index, int end_index, const char* content) {
        if (start_index < 0 || end_index > m_final_index || start_index > end_index || content == nullptr) {
            return -1;
        }

        if (start_index == end_index) return 0;

        int i;
        for (i = start_index; i < end_index && *content != '\0'; i++, content++) {
            m_string[i] = *content;
        }

        bool truncated = (i == end_index) && (*content != '\0');
        if (truncated) m_is_truncated = true;

        return truncated ? 1 : 0;
    }

    const char* c_str() const { return m_string; }
};