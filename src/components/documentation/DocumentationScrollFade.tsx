export default function DocumentationScrollFade() {
  return (
    <>
      <div className="pointer-events-none absolute left-0 right-0 top-0 z-20 h-20 bg-gradient-to-b from-[#0c0c0c] to-transparent sm:h-24" />

      <div className="pointer-events-none absolute bottom-0 left-0 right-0 z-20 h-20 bg-gradient-to-t from-[#0c0c0c] to-transparent sm:h-24" />
    </>
  );
}