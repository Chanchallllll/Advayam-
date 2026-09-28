export function runSiteSearch(query: string) {
  const value = query.trim().toLowerCase();
  if (!value) return;

  document.querySelectorAll(".search-highlight").forEach((el) => {
    el.classList.remove("search-highlight");
  });

  const elements = Array.from(
    document.querySelectorAll<HTMLElement>(
      "h1,h2,h3,h4,p,a,span,li,td,th,button,label"
    )
  ).filter((el) => el.innerText?.trim());

  const match = elements.find((el) =>
    el.innerText.toLowerCase().includes(value)
  );

  if (!match) {
    alert(`No result found for "${query}"`);
    return;
  }

  match.classList.add("search-highlight");

  const pageScrollable =
    document.documentElement.scrollHeight > window.innerHeight + 20;

  if (pageScrollable) {
    match.scrollIntoView({
      behavior: "smooth",
      block: "center",
    });
  }
}