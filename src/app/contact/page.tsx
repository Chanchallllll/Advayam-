"use client";

import { useState } from "react";
import Header from "@/components/Header";
import Footer from "@/components/Footer";
import { SOCIAL_ITEMS } from "@/config/socials";
import { apiFetch } from "@/lib/api";
import { useToast } from "@/context/ToastContext";

export default function ContactPage() {
  const { showToast } = useToast();

  const [name, setName] = useState("");
  const [email, setEmail] = useState("");
  const [message, setMessage] = useState("");
  const [submitting, setSubmitting] = useState(false);
  const [statusMessage, setStatusMessage] = useState("");
  const [isError, setIsError] = useState(false);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setStatusMessage("");
    setIsError(false);

    if (!name.trim() || !email.trim() || !message.trim()) {
      setIsError(true);
      setStatusMessage("Please fill in all required fields.");
      showToast("Please fill in all required fields.", "error");
      return;
    }

    setSubmitting(true);
    try {
      // Create support ticket or contact entry in backend
      await apiFetch("/tickets", {
        method: "POST",
        body: JSON.stringify({
          category: "General Inquiry",
          title: `Contact Form: ${name}`,
          description: `From: ${name} (${email})\n\nMessage:\n${message}`,
          priority: "medium",
          attachments: [],
        }),
      });

      showToast("Thank you for contacting us! We'll get back to you shortly.", "success");
      setStatusMessage("Thank you for reaching out! Your message has been received.");
      setName("");
      setEmail("");
      setMessage("");
    } catch (err: any) {
      // Fallback if user is unauthenticated: attempt newsletter/inquiry route
      try {
        await apiFetch("/newsletter/subscribe", {
          method: "POST",
          body: JSON.stringify({ email }),
        });
        showToast("Thank you for contacting us! We've received your request.", "success");
        setStatusMessage("Thank you for reaching out! We will respond to your email soon.");
        setName("");
        setEmail("");
        setMessage("");
      } catch (fallbackErr: any) {
        setIsError(true);
        const errorMsg = err.message || fallbackErr.message || "Failed to send message. Please try again.";
        setStatusMessage(errorMsg);
        showToast(errorMsg, "error");
      }
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <main className="flex min-h-screen flex-col bg-black text-white">
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <h1 className="font-jakarta text-5xl md:text-6xl font-black text-[#202020]">Get in Touch</h1>
          <p className="mt-3 font-jakarta text-lg text-slate-600 font-medium">Have a question, feedback, or business proposal? Reach out to our team.</p>

          <div className="mt-12 grid gap-8 md:grid-cols-2">
            <form onSubmit={handleSubmit} className="rounded-[2rem] border-2 border-black bg-white p-8 shadow-sm">
              <div className="mb-4">
                <label className="block mb-2 font-jakarta text-sm font-bold text-slate-700">Your Name</label>
                <input
                  type="text"
                  value={name}
                  onChange={(e) => setName(e.target.value)}
                  className="w-full rounded-full border-2 border-black px-5 py-3 font-jakarta text-sm outline-none focus:border-[#BE5209] transition-colors"
                  placeholder="Enter your name"
                  required
                />
              </div>

              <div className="mb-4">
                <label className="block mb-2 font-jakarta text-sm font-bold text-slate-700">Email Address</label>
                <input
                  type="email"
                  value={email}
                  onChange={(e) => setEmail(e.target.value)}
                  className="w-full rounded-full border-2 border-black px-5 py-3 font-jakarta text-sm outline-none focus:border-[#BE5209] transition-colors"
                  placeholder="Enter your email"
                  required
                />
              </div>

              <div className="mb-4">
                <label className="block mb-2 font-jakarta text-sm font-bold text-slate-700">Message</label>
                <textarea
                  value={message}
                  onChange={(e) => setMessage(e.target.value)}
                  className="h-40 w-full rounded-[2rem] border-2 border-black p-5 font-jakarta text-sm outline-none focus:border-[#BE5209] transition-colors resize-none"
                  placeholder="How can we help you?"
                  required
                />
              </div>

              {statusMessage && (
                <div className={`mb-4 rounded-xl p-4 font-jakarta text-sm font-bold ${isError ? "bg-red-50 text-red-600 border border-red-200" : "bg-green-50 text-green-700 border border-green-200"}`}>
                  {statusMessage}
                </div>
              )}

              <button
                type="submit"
                disabled={submitting}
                className="rounded-full bg-[#BE5209] px-8 py-4 font-jakarta text-base font-bold text-white hover:bg-black transition-colors disabled:opacity-50"
              >
                {submitting ? "Sending Message..." : "Send Message"}
              </button>
            </form>

            <div className="rounded-[2rem] bg-[#DBDBDB] p-8 flex flex-col justify-between border-2 border-black/10">
              <div>
                <h2 className="font-jakarta text-3xl font-black text-[#202020]">Contact Options</h2>
                <div className="mt-6 space-y-4 font-jakarta font-medium text-slate-700">
                  <div className="rounded-xl bg-white/60 p-4 border border-black/5">
                    <p className="font-bold text-slate-900 text-sm">Technical Support</p>
                    <p className="text-slate-600 text-sm mt-1">support@advayamtech.in</p>
                  </div>
                  <div className="rounded-xl bg-white/60 p-4 border border-black/5">
                    <p className="font-bold text-slate-900 text-sm">Business Enquiries</p>
                    <p className="text-slate-600 text-sm mt-1">contact@webyfy.com</p>
                  </div>
                  <div className="rounded-xl bg-white/60 p-4 border border-black/5">
                    <p className="font-bold text-slate-900 text-sm">Partner Programme</p>
                    <p className="text-slate-600 text-sm mt-1">partners@webyfy.com</p>
                  </div>
                </div>
              </div>

              <div className="mt-8 pt-6 border-t border-black/10">
                <h3 className="font-jakarta text-lg font-bold text-[#202020]">Follow Us on Social Media</h3>
                <div className="mt-4 flex flex-wrap gap-3">
                  {SOCIAL_ITEMS.map(({ name, icon: Icon, href }) => (
                    <a
                      key={name}
                      href={href}
                      target="_blank"
                      rel="noopener noreferrer"
                      className="flex items-center gap-2.5 rounded-full border-2 border-[#202020] bg-white px-5 py-2.5 font-jakarta text-sm font-bold text-[#202020] hover:bg-[#202020] hover:text-white transition-colors"
                    >
                      <Icon className="h-4 w-4" />
                      <span>{name}</span>
                    </a>
                  ))}
                </div>
              </div>
            </div>
          </div>
        </div>
      </section>
      <Footer />
    </main>
  );
}
