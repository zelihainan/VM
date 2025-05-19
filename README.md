# Project1


---

## 🔧 Gereken Kütüphaneler

Tüm kütüphaneler `Libraries/` klasöründe hazır şekilde yüklüdür:

---

## 🛠️ Kurulum (Visual Studio)

1. GitHub üzerinden projeyi indir:
    ```bash
    git clone https://github.com/zelihainan/Project1.git
    ```

2. Visual Studio ile `.sln` dosyasını aç.

3. Şu yolları `Project Properties` kısmına ekle:
   - **C/C++ > Additional Include Directories**:
     ```
     $(ProjectDir)Libraries\include
     ```
   - **Linker > Additional Library Directories**:
     ```
     $(ProjectDir)Libraries\lib
     ```

4. `imgui_impl_glfw.cpp`, `imgui_impl_opengl3.cpp` dosyalarını projeye ekle:  
   **Solution Explorer > Source Files > Add > Existing Item**

5. Projeyi derle ve çalıştır.

---

