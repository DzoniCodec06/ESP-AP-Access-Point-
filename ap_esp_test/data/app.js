const textBtn = document.getElementById("txtBtn");
const solidBtn = document.getElementById("solidBtn");

const container = document.getElementById("cont");

textBtn.addEventListener("click", () => {
    textBtn.classList.replace("uab", "ab");
    solidBtn.classList.replace("ab", "uab");
});

solidBtn.addEventListener("click", () => {
    textBtn.classList.replace("ab", "uab");
    solidBtn.classList.replace("uab", "ab");
});